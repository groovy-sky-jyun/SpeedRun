// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourComponent.h"
#include "SpeedRunCharacter.h"
#include "ParkourMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "MotionWarpingComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetTextLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "ParkourBlock.h"
#include "Chooser.h"
#include "ChooserFunctionLibrary.h"    
#include "IObjectChooser.h" 
#include "InstancedStruct.h"


UParkourComponent::UParkourComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UParkourComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<ASpeedRunCharacter>(GetOwner());

	if (!Player) return;

	AnimInstance = Player->GetMesh()->GetAnimInstance();
	WarpComponent = Player->GetMotionWarpingComponent();
	ParkourMovement = Player->GetParkourMovement();
}


void UParkourComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


//=================================
//      액션 수행 (Execution)         
//=================================
void UParkourComponent::PerformJumpSequence()
{
	CurrentEnvData = {};

	bCanParkour = TryUpdateEnvData();
	if (!bCanParkour)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't Parkour Jump"));
		Player->Jump();
		return;
	}

	CurrentParkourAction = EvaluateNextAction(CurrentEnvData);
	switch (CurrentParkourAction){
	case EParkourActionType::Hurdle:
		ExecuteMontageByActionType(CurrentParkourAction, CurrentEnvData);
		break;
	case EParkourActionType::Vault:
		ExecuteMontageByActionType(CurrentParkourAction, CurrentEnvData);
		break;
	case EParkourActionType::Hang:
		break;
	case EParkourActionType::WallRun:
		break;
	default:
		return;
	}

}

bool UParkourComponent::TryUpdateEnvData()
{
	FVector ActorLocation = Player->GetActorLocation();
	FVector ActorForward = Player->GetActorForwardVector();
	float CapsuleRadius = Player->GetCapsuleComponent()->GetScaledCapsuleRadius();
	float CapsuleHalfHeight = Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	// 1.정면 장애물 감지
	FHitResult ObstacleHitResult = TryDetectObstacle(ActorLocation, ActorForward, CapsuleHalfHeight);

	// 2.장애물 미감지 -> Jump or StepBoxJump
	if (!ObstacleHitResult.bBlockingHit)
	{
		// 2.1.Detect Step Box
		FHitResult StepBoxLastHitResult = TryDetectStepBox(ActorLocation, ActorForward, CapsuleHalfHeight);

		if (!StepBoxLastHitResult.bBlockingHit) //낭떨어지 직전 위치 반환
		{
			UE_LOG(LogTemp, Warning, TEXT("NON Detect Anyone. Just Jump"));
			return false;
			
		}

		// 2.2.Detect Next Step Box
		return UpdateStepBoxData(StepBoxLastHitResult.ImpactPoint, 15.f, CurrentEnvData, ActorForward);
	}

	// 3.장애물이 ParkourBlock이 아닌 경우 일반 Jump
	AParkourBlock* Block = Cast<AParkourBlock>(ObstacleHitResult.GetActor());

	if (Block == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Detect Obstacle. But It's not ParkourBlock"));
		return false;
	}

	// 4.감지된 Obstacle Data Update. 
	return UpdateObstacleData(ObstacleHitResult, Block, CurrentEnvData, ActorLocation, CapsuleRadius, CapsuleHalfHeight);
}

EParkourActionType UParkourComponent::EvaluateNextAction(const FEnvironmentData& InCurrentEnvData)
{
	return EParkourActionType::None;
}

void UParkourComponent::ExecuteMontageByActionType(const EParkourActionType ActionType, const FEnvironmentData& InCurrentEnvData)
{
	// 1.ActionType에 맞는 ChooserTable 찾기
	UChooserTable* CHT = ActionToCHT.FindRef(ActionType);

	if(!CHT)
	{
		FString ActionName = UEnum::GetValueAsString(ActionType);
		UE_LOG(LogTemp, Warning, TEXT("%s to CHT is null"), *ActionName);

		Player->Jump();

		return;
	}

	// 2.ChooserTable 실행 -> 알맞은 AnimMontage 얻기
	UAnimMontage* CurrentMontage = SelectActionMontageFromCHT(CHT, InCurrentEnvData);

	if (!CurrentMontage)
	{
		FString ActionName = UEnum::GetValueAsString(ActionType);
		UE_LOG(LogTemp, Warning, TEXT("%s AnimMontage is null"), *ActionName);

		Player->Jump();

		return;
	}

	SetupMotionWarping();
}

UAnimMontage* UParkourComponent::SelectActionMontageFromCHT(UChooserTable* CHT, const FEnvironmentData& InCurrentEnvData)
{
	if (!CHT || !Player)
	{
		return nullptr;
	}

	// 1.Update Strut:TraversalChooserParams 
	FTraversalChooserParams ChooserData = {};
	ChooserData.UpdateTraversalChooserParams(InCurrentEnvData, Player);

	// 2.Add FChooserEvaluationContext Params (CHT에 넘겨줄 내용물)
	FChooserEvaluationContext ChooserContext;
	ChooserContext.AddStructParam(ChooserData);
	ChooserContext.AddObjectParam(Player);

	// 3.Change ChooserTable To InstancedStruct (ChooserTable을 실행 가능한 Struct 구조로 변경)
	FInstancedStruct ChooserStruct = UChooserFunctionLibrary::MakeEvaluateChooser(CHT);

	// 4.Execute ChooserTable
	UObject* Result = UChooserFunctionLibrary::EvaluateObjectChooserBase(
		ChooserContext, //Chooser evaluation context
		ChooserStruct, //EvaluateProxyAsset
		UAnimMontage::StaticClass() //return Object
	);

	return Cast<UAnimMontage>(Result);
}

void UParkourComponent::SetupMotionWarping() const
{
	UE_LOG(LogTemp, Warning, TEXT("Motion Warping Success"));
}


//=================================
//           환경 감지 
//=================================
FHitResult UParkourComponent::TryDetectObstacle(FVector ActorLocation, FVector ActorForward, float CapsuleHalfHeight)
{
	FHitResult HitResult;

	FVector Start = ActorLocation + FVector(0.f, 0.f, -CapsuleHalfHeight + 40.f);
	FVector End = Start + ActorForward * 300.f;
	HitResult = LineTrace(Start, End, true);

	return HitResult;
}

bool UParkourComponent::UpdateObstacleData(FHitResult ObstacleHitResult, AParkourBlock* Block, FEnvironmentData& TraversalResult, FVector ActorLocation, float CapsuleRadius, float CapsuleHalfHeight)
{
	// 1.Hit 된 Obstacle Data 담기
	TraversalResult = Block->GetLedgeTransform(ObstacleHitResult.ImpactPoint, Player->GetActorLocation()); 
	TraversalResult.HitComponent = ObstacleHitResult.GetComponent();

	// 2.감지된 Obstacle의 FrontLedgeData가 Traversal 조건에 부합하지 않은 경우 종료
	FVector FrontLedgeLocation = TraversalResult.Obstacle_Data.FrontLedgeLocation;
	FVector BackLedgeLocation = TraversalResult.Obstacle_Data.BackLedgeLocation;
	if (!TraversalResult.Obstacle_Data.bHasFrontLedge)
	{
		UE_LOG(LogTemp, Warning, TEXT("Detect Obstacle. But It hasn't FrontLedge."));
		return false;
	}
	else
	{
		DrawSphereTrace(FrontLedgeLocation, 10.f, 5.0f);
	}

	// 3.점프하는 궤도에 장애물 없는지 체크 (FrontLedge)
	FVector FrontLedgeNormal = TraversalResult.Obstacle_Data.FrontLedgeNormal;

	FVector FrontLedgeTopSurfaceLocation = FrontLedgeLocation + FVector(0.f, 0.f, CapsuleHalfHeight) + (-FrontLedgeNormal * (CapsuleRadius/2));

	float StartZOffset = FrontLedgeTopSurfaceLocation.Z - CapsuleHalfHeight/2;
	FVector Start = FVector(ActorLocation.X, ActorLocation.Y, StartZOffset);

	FHitResult FrontLedgeRoomHitResult = CapsuleTrace(Start, FrontLedgeTopSurfaceLocation, CapsuleRadius / 2, CapsuleHalfHeight/2, true);
	if (FrontLedgeRoomHitResult.bBlockingHit | FrontLedgeRoomHitResult.bStartPenetrating) //장애물 있다면 일반 Jump
	{
		TraversalResult.Obstacle_Data.bHasFrontLedge = false;
		UE_LOG(LogTemp, Warning, TEXT("Detect Obstacle. But It hasn't Surface."));
		return false;
	}

	// 4.Obstacle Height Data 저장 (FrontLedge)
	float ObstacleHeight = FMath::Abs((ActorLocation.Z - CapsuleHalfHeight) - FrontLedgeLocation.Z);
	TraversalResult.Obstacle_Data.ObstacleHeight = ObstacleHeight;
	UE_LOG(LogTemp, Warning, TEXT("Obstacle Height : %f"), ObstacleHeight);


	// 5.감지된 Obstacle의 BackLedgeData가 Traversal 조건에 부합하지 않은 경우 종료
	if (!TraversalResult.Obstacle_Data.bHasBackLedge)
	{
		UE_LOG(LogTemp, Warning, TEXT("Detect Obstacle. But It hasn't BackLedge."));
		return false;
	}
	else
	{
		DrawSphereTrace(BackLedgeLocation, 10.f, 5.0f);
	}

	// 6.물체 Depth || BackFloor 체크
	FVector BackLedgeNormal = TraversalResult.Obstacle_Data.BackLedgeNormal;
	FVector BackLedgeTopSurfaceLocation = BackLedgeLocation + FVector(0.f, 0.f, CapsuleHalfHeight + 2.f) + (BackLedgeNormal * (CapsuleRadius + 2.0));
	FHitResult TopSweepResult = CapsuleTrace(FrontLedgeTopSurfaceLocation, BackLedgeTopSurfaceLocation, CapsuleRadius, CapsuleHalfHeight, false);
		// 6.1.BackLedge까지 장애물이 없는 경우 : BackLedge까지를 Depth로 지정
	if (!TopSweepResult.bBlockingHit) 
	{
		// 6.1.1.Obstacle Depth Data 저장
		FVector ObstacleDepthVector = FrontLedgeLocation - BackLedgeLocation;
		float ObstacleDepth = ObstacleDepthVector.Size2D();
		TraversalResult.Obstacle_Data.ObstacleDepth = ObstacleDepth;
		UE_LOG(LogTemp, Warning, TEXT("Obstacle Depth : %f"), ObstacleDepth);

		// 6.1.2.Back Floor Data 저장
		FVector FloorFrontLocation = BackLedgeLocation + (BackLedgeNormal * (CapsuleRadius + 2.0));
		float FloorZOffset = ObstacleHeight + 50.f;
		FVector EndLocation = FloorFrontLocation - FVector(0.f, 0.f, FloorZOffset) + FVector(0.f, 0.f, CapsuleHalfHeight); //플레이어 중심이 바닥에 위치하면 안되므로 CapsuleHalfHeight를 더해준다.
		FHitResult SurfaceHitResult = CapsuleTrace(BackLedgeTopSurfaceLocation, EndLocation, CapsuleRadius, CapsuleHalfHeight, false);

		if (SurfaceHitResult.bBlockingHit && !SurfaceHitResult.bStartPenetrating) // 착지 지점 있는 경우
		{
			TraversalResult.Obstacle_Data.bHasBackFloor = true;
			TraversalResult.Obstacle_Data.BackFloorLocation = SurfaceHitResult.ImpactPoint;
			TraversalResult.Obstacle_Data.BackLedgeHeight = (float)FMath::Abs(SurfaceHitResult.ImpactPoint.Z - BackLedgeLocation.Z);
			UE_LOG(LogTemp, Warning, TEXT("Obstacle Back Ledge Height : %f"), TraversalResult.Obstacle_Data.BackLedgeHeight);
		}
		else // 착지 지점 없는 경우
		{
			TraversalResult.Obstacle_Data.bHasBackFloor = false;
		}
	}
	else // 6.2.BackLedge 전에 장애물이 감지된 경우 : 장애물까지를 Depth로 지정
	{
		FVector ObstacleDepthVector = TopSweepResult.ImpactPoint - FrontLedgeLocation;
		float ObstacleDepth = ObstacleDepthVector.Size2D();
		TraversalResult.Obstacle_Data.ObstacleDepth = ObstacleDepth;
		TraversalResult.Obstacle_Data.bHasBackLedge = false;
		UE_LOG(LogTemp, Warning, TEXT("Obstacle Depth : %f"), ObstacleDepth);
	}
	return true;
}


FHitResult UParkourComponent::TryDetectStepBox(FVector ActorLocation, FVector ActorForward, float CapsuleHalfHeight)
{
	int32 TraceCount = 10;
	float Radius = 15.f;
	float TraceDistance = 50.f;
	FVector Start = ActorLocation - FVector(0.f, 0.f, CapsuleHalfHeight + TraceDistance / 2);

	FHitResult HitResult = ScanSurfaceEdge(ETraceDirection::Vertical, TraceCount, Start, ActorForward, TraceDistance, Radius * 2, Radius, false, true);

	return HitResult;
}

bool UParkourComponent::UpdateStepBoxData(FVector EdgeLocation, float Radius, FEnvironmentData& TraversalResult, FVector ActorForward)
{
	// 1.건너편 StepBox 존재 유무 확인
	FVector Start = EdgeLocation - FVector(0.f, 0.f, Radius) + (ActorForward * Radius);
	FHitResult NextStepBoxHitResult = LineTrace(Start, Start + (ActorForward * MaxStepBoxGapDistance), true);

	// 2.ParkourBlock이 아니라면 종료 
	AParkourBlock* Block = Cast<AParkourBlock>(NextStepBoxHitResult.GetActor());
	if (Block == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("NextStepBox isn't ParkourBlock"));
		TraversalResult.StepBox_Data.bIsOnEdge = true;
		return false;
	}

	// 3.Update StepBoxData 
	TraversalResult = Block->GetLedgeTransformToStepBox(NextStepBoxHitResult.ImpactPoint);
	TraversalResult.HitComponent = NextStepBoxHitResult.GetComponent();
	TraversalResult.StepBox_Data.bIsOnEdge = true;
	TraversalResult.StepBox_Data.GapDepth = FVector::Dist(EdgeLocation, TraversalResult.StepBox_Data.NextFrontLedgeLocation);

	UE_LOG(LogTemp, Warning, TEXT("Step Box Gap : %f"), TraversalResult.StepBox_Data.GapDepth);
	
	return true;
}

FHitResult UParkourComponent::ScanSurfaceEdge(ETraceDirection TraceDir, int32 Count, FVector Start, FVector Dir, float Distance, float GapSize, float Radius, bool bReturnHit, bool bDrawDebug) const
{
	FHitResult LastHitResult;

	for (int i = 0; i < Count; i++)
	{
		FVector StartLocation, EndLocation;

		if (TraceDir == ETraceDirection::Horizontal)
		{
			StartLocation = Start +  FVector(0.f, 0.f, GapSize * i);
			EndLocation = StartLocation + Dir * Distance;
		}
		else
		{

			StartLocation = Start + (Dir * (GapSize * i));
			EndLocation = StartLocation + FVector(0.f, 0.f, Distance);
		}

		FHitResult HitResult = SphereTrace(StartLocation, EndLocation, Radius, bDrawDebug);

		if (bReturnHit) //immediately return if find surface to floor.
		{
			if (HitResult.bBlockingHit) return HitResult;
		}
		else { //낭떨어지 직전의 바닥 위치 반환 (낭떨어지 감지)
			if (HitResult.bBlockingHit)
			{
				if (i == Count - 1) //낭떨어지를 감지하지 못한 경우
				{
					return FHitResult();
				}
				LastHitResult = HitResult;
			}
			else
			{
				return LastHitResult;
			}
		}
	}
	return FHitResult();
}


//=================================
//      Basic Trace Logic
//=================================
FHitResult UParkourComponent::SphereTrace(const FVector& Start, const FVector& End, float Radius, bool bDrawDebug) const
{
	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Player);

	UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(),
		Start,
		End,
		Radius,
		UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), // Trace Channel
		false,         // Trace Complex
		ActorsToIgnore,
		bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, // Draw Debug Type
		HitResult,
		true,          // Ignore Self
		FLinearColor::Red,   // 디버그 선 색상
		FLinearColor::Green, // 히트 시 색상
		5.0f           // 디버그 선 유지 시간
	);
	return HitResult;
}


FHitResult UParkourComponent::BoxTrace(FVector Start, FVector End, FVector BoxHalfSize, FRotator Rotation, bool bDrawDebug) const
{
	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Player);

	UKismetSystemLibrary::BoxTraceSingle(
		GetWorld(),
		Start,
		End,
		BoxHalfSize, //FVector(가로, 세로, 높이)
		Rotation,
		UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1),
		false,
		ActorsToIgnore,
		bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		HitResult,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		5.0f
	);

	return HitResult;
}

FHitResult UParkourComponent::LineTrace(FVector Start, FVector End, bool bDrawDebug) const
{
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Player);
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_GameTraceChannel1, Params);

	if (bDrawDebug)
	{
		DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 3.f);
	}

	return HitResult;
}

FHitResult UParkourComponent::CapsuleTrace(FVector& Start, FVector& End, float Radius, float HalfHeight, bool bDrawDebug) const
{
	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Cast<AActor>(Player));

	// SphereTraceSingle 대신 CapsuleTraceSingle을 사용합니다.
	UKismetSystemLibrary::CapsuleTraceSingle(
		GetWorld(),
		Start,
		End,
		Radius,
		HalfHeight, 
		UEngineTypes::ConvertToTraceType(ECC_Visibility), //카메라 충돌 여부
		false,         // Trace Complex
		ActorsToIgnore,
		bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, 
		HitResult,
		true,          // Ignore Self
		FLinearColor::Red,   // 디버그 선 색상
		FLinearColor::Green, // 히트 시 색상
		5.0f           // 디버그 선 유지 시간
	);

	return HitResult;
}

void UParkourComponent::DrawSphereTrace(FVector Center, float Radius, float LifeTime) const
{
	DrawDebugSphere(
		GetWorld(),
		Center,
		Radius,             
		12,                 // Segments (얼마나 동그랗게 보일지)
		FColor::Cyan,       
		false,              // bPersistentLines (영구적으로 남길지 여부)
		LifeTime,              
		0,                  // DepthPriority
		1.0f                // Thickness 
	);
}

void FTraversalChooserParams::UpdateTraversalChooserParams(const FEnvironmentData& CheckResult, ACharacter* Player)
{
	if (!Player) return;

	// 1.Update Player Movement Info
	MovementMode = Player->GetCharacterMovement()->MovementMode;
	Speed = Player->GetVelocity().Size2D();

	// 2.Update Obstacle Info
	bHasFrontLedge = CheckResult.Obstacle_Data.bHasFrontLedge;
	bHasUpperSurface = CheckResult.Obstacle_Data.bHasUpperSurface;
	bHasBackFloor = CheckResult.Obstacle_Data.bHasBackFloor;
	ObstacleHeight = CheckResult.Obstacle_Data.ObstacleHeight;
	ObstacleDepth = CheckResult.Obstacle_Data.ObstacleDepth;

	// 3.Update Step Box Info
	bIsOnEdge = CheckResult.StepBox_Data.bIsOnEdge;
	GapDepth = CheckResult.StepBox_Data.GapDepth;
}
