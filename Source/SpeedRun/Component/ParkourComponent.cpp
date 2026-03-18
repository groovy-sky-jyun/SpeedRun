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

	CapsuleRadius = Player->GetCapsuleComponent()->GetScaledCapsuleRadius();
	CapsuleHalfHeight = Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
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
	// EnvData Reset
	FEnvData EnvData = {};

	// 1.Update EnvData & Check Can Parkour
	bCanParkour = TryUpdateEnvData(EnvData);
	if (!bCanParkour)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't Parkour Jump"));
		Player->Jump();
		return;
	}

	// 2.Evaluate ActionType by EnvData
	CurrentAction = EvaluateNextAction(EnvData);
	FString ActionName = UEnum::GetValueAsString(CurrentAction);
	UE_LOG(LogTemp, Warning, TEXT("Current Action : %s"), *ActionName);

	if(CurrentAction == EParkourActionType::PARKOUR_None)
	{
		Player->Jump();
		return;
	}

	// 3.Play Montage with CHT, MotionWarping
	ExecuteMontageByActionType(CurrentAction, EnvData);
}

bool UParkourComponent::TryUpdateEnvData(FEnvData& EnvData)
{
	FVector ActorLocation = Player->GetActorLocation();
	FVector ActorForward = Player->GetActorForwardVector();
	
	FHitResult ObstacleHitResult = TryDetectObstacle(ActorLocation, ActorForward);

	// 1.not found Obstacle
	if (!ObstacleHitResult.bBlockingHit)
	{
		FHitResult StepBoxLastHitResult = TryDetectStepBox(ActorLocation, ActorForward);

		// 1.1.not found Step Box -> Jump
		if (!StepBoxLastHitResult.bBlockingHit)
		{
			return false; 
		}

		// 1.2.found Step Box -> Update StepBoxData
		return UpdateStepBoxData(StepBoxLastHitResult.ImpactPoint, 15.f, EnvData, ActorForward);
	}

	// 2.found Obstacle
	AParkourBlock* Block = Cast<AParkourBlock>(ObstacleHitResult.GetActor());

	if (Block == nullptr) // 2.1.Check Obstacle is ParkourBlock.
	{
		UE_LOG(LogTemp, Warning, TEXT("Obstacle isn't ParkourBlock"));
		return false;
	}

	// 2.2.Update ObstacleData
	return UpdateObstacleData(ObstacleHitResult, Block, EnvData, ActorLocation);
}

EParkourActionType UParkourComponent::EvaluateNextAction(const FEnvData& InEnvData)
{
	/* Choose New ParkourActionType based on the EnvData*/

	EMovementMode MovementMode = Player->GetCharacterMovement()->MovementMode;

	switch (MovementMode) {
	case EMovementMode::MOVE_Walking:
		// Hasn't FrontLedge -> Hurdle/Jump
		if (!InEnvData.Obstacle_Data.bHasFrontLedge)
		{
			if (InEnvData.StepBox_Data.bHasLandingSurface)
			{
				return EParkourActionType::PARKOUR_Hurdle;
			}
		}

		if (InEnvData.Obstacle_Data.FrontHeight < 50.f)
		{
			return EParkourActionType::PARKOUR_None;
		}

		// Has FrontLedge && Has UpperSurface -> Vault/Mantle/Hang
		if (InEnvData.Obstacle_Data.bHasLandingSurface)
		{
			if (InEnvData.Obstacle_Data.FrontHeight <= MaxObstacleHeight_Vault)
			{
				return EParkourActionType::PARKOUR_Vault;
			}
		}
		
		if(InEnvData.Obstacle_Data.bHasUpperSurface)
		{
			if (InEnvData.Obstacle_Data.FrontHeight <= MaxObstacleHeight_Mantle)
			{
				return EParkourActionType::PARKOUR_Mantle;
			}
		}

		if (MaxObstacleHeight_Hang <= MaxObstacleHeight_Hang)
		{
			return EParkourActionType::PARKOUR_Hang;
		}

		break;
	case EMovementMode::MOVE_Falling: // hang
		if (InEnvData.Obstacle_Data.bHasFrontLedge) return EParkourActionType::PARKOUR_Hang;
		break;
	case EMovementMode::MOVE_Flying:
		break;
	case EMovementMode::MOVE_None:
		break;
	}


	return EParkourActionType::PARKOUR_None;
}

void UParkourComponent::ExecuteMontageByActionType(const EParkourActionType ActionType, const FEnvData& InEnvData)
{
	// 1.Find Chooser Table by ParkourActionType
	UChooserTable* CHT = ActionToCHT.FindRef(ActionType);

	if(!CHT)
	{
		FString ActionName = UEnum::GetValueAsString(ActionType);
		UE_LOG(LogTemp, Warning, TEXT("%s to CHT is null"), *ActionName);

		Player->Jump();

		return;
	}

	// 2.Find AnimMontage from CHT
	UAnimMontage* CurrentMontage = SelectActionMontageFromCHT(CHT, InEnvData);

	if (!CurrentMontage)
	{
		FString ActionName = UEnum::GetValueAsString(ActionType);
		UE_LOG(LogTemp, Warning, TEXT("%s AnimMontage is null"), *ActionName);

		Player->Jump();

		return;
	}

	// 3.Set and Play Animation
	SetupMotionWarping(ActionType, InEnvData);

	Player->PlayAnimMontage(CurrentMontage);

	/* 후에 추가
	// 지속성 액션 물리 제어 (중력 끄기)
	if (ActionType == EParkourActionType::PARKOUR_Hang || ActionType == EParkourActionType::PARKOUR_WallRun)
	{
	  Player->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	  Player->GetCharacterMovement()->StopMovementImmediately();
		}
	*/
}

UAnimMontage* UParkourComponent::SelectActionMontageFromCHT(UChooserTable* CHT, const FEnvData& InEnvData)
{
	if (!CHT || !Player)
	{
		return nullptr;
	}

	// 1.Update Strut:TraversalChooserParams 
	FTraversalChooserParams ChooserData = {};
	ChooserData.UpdateTraversalChooserParams(InEnvData, Player);

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

void UParkourComponent::SetupMotionWarping(const EParkourActionType ActionType, const FEnvData& InEnvData)
{
	if (!WarpComponent) return;

	WarpComponent->RemoveAllWarpTargets();
	const FObstacleData& ObstacleData = InEnvData.Obstacle_Data;
	const FStepBoxData& StepBoxData = InEnvData.StepBox_Data;

	switch (ActionType){
	case EParkourActionType::PARKOUR_Vault:
		if (ObstacleData.bHasFrontLedge)
		{
			AddWarpTarget(FName("FrontEdge"), ObstacleData.FrontLedgeLocation, ObstacleData.FrontLedgeNormal);
		}
		if (ObstacleData.bHasLandingSurface)
		{
			AddWarpTarget(FName("DropLanding"), ObstacleData.LandingSurfaceLocation, ObstacleData.FrontLedgeNormal);
		}
		break;
	case EParkourActionType::PARKOUR_Hurdle:
		if (StepBoxData.bHasNextFrontLedge)
		{
			AddWarpTarget(FName("Landing"), StepBoxData.LandingSurfaceLocation, StepBoxData.NextFrontLedgeNormal);
		}
		break;
	case EParkourActionType::PARKOUR_Mantle:
		if (ObstacleData.bHasFrontLedge)
		{
			AddWarpTarget(FName("FrontEdge"), ObstacleData.FrontLedgeLocation, ObstacleData.FrontLedgeNormal);
		}
		if (ObstacleData.bHasLandingSurface)
		{
			AddWarpTarget(FName("UpperLanding"), ObstacleData.UpperSurfaceLocation, ObstacleData.FrontLedgeNormal);
		}
		break;
	case EParkourActionType::PARKOUR_Hang:
		if (ObstacleData.bHasFrontLedge)
		{
			// 손을 짚을 앞쪽 모서리(FrontEdge) 위치와 노멀 전달
			AddWarpTarget(FName("FrontEdge"), ObstacleData.FrontLedgeLocation, ObstacleData.FrontLedgeNormal);
		}
		break;
	default:
		break;
	}
}

void UParkourComponent::AddWarpTarget(FName TargetName, FVector Location, FVector Normal)
{
	FRotator TargetRotation = (-Normal).Rotation();
	// Yaw 만 적용
	TargetRotation.Pitch = 0.f;
	TargetRotation.Roll = 0.f;

	WarpComponent->AddOrUpdateWarpTargetFromLocationAndRotation(TargetName, Location, TargetRotation);
}


//=================================
//           환경 감지 
//=================================
FHitResult UParkourComponent::TryDetectObstacle(FVector ActorLocation, FVector ActorForward)
{
	FHitResult HitResult;

	FVector Start = ActorLocation + FVector(0.f, 0.f, -CapsuleHalfHeight + 40.f);
	FVector End = Start + ActorForward * 300.f;
	HitResult = LineTrace(Start, End, true);

	return HitResult;
}

FHitResult UParkourComponent::TryDetectStepBox(FVector ActorLocation, FVector ActorForward)
{
	int32 TraceCount = 10;
	float Radius = 15.f;
	float TraceDistance = 50.f;
	FVector Start = ActorLocation - FVector(0.f, 0.f, CapsuleHalfHeight + TraceDistance / 2);

	//낭떨어지 직전의 바닥 위치 반환 (낭떨어지 감지)
	FHitResult HitResult = ScanSurfaceEdge(ETraceDirection::Vertical, TraceCount, Start, ActorForward, TraceDistance, Radius * 2, Radius, false, true);

	return HitResult;
}


bool UParkourComponent::UpdateObstacleData(FHitResult ObstacleHitResult, AParkourBlock* Block, FEnvData& EnvData, FVector ActorLocation)
{
	/* return false mean "can't parkour, just jump." */

	/* ----- [Update List] ----- */
	/* FObstacleData : UpperSurface Data */
	/* FObstacleData : LandingSurface Data */
	/* FObstacleData : Obstacle Value */

	// 1.Update Front/BackLedge Transform by ParkourBlock
	EnvData.Obstacle_Data = Block->UpdateObstacleData(ObstacleHitResult.ImpactPoint, Player->GetActorLocation());
	EnvData.HitComponent = ObstacleHitResult.GetComponent();
	FObstacleData& ObsData = EnvData.Obstacle_Data; // notice::It has just Front/Back Ledge Transform.


	// 2.Check It has FrontLedge
	if (!ObsData.bHasFrontLedge)
	{
		UE_LOG(LogTemp, Warning, TEXT("Obstacle hasn't FrontLedge."));
		return false;
	}
	else
	{
		DrawSphereTrace(ObsData.FrontLedgeLocation, 5.f, 5.0f);
	}


	// 3.Update UpperSurface Data (+Check if space to the upper surface is clear)
	float ZOffset = 5.f + CapsuleHalfHeight;
	FVector UpperSurfaceLocation = ObsData.FrontLedgeLocation + (-ObsData.FrontLedgeNormal * (CapsuleRadius + 5.f)); 
	FVector UpperSurfacePlusOffset = UpperSurfaceLocation + FVector(0.f, 0.f, ZOffset);
	FVector FrontLedgePlusOffset = ObsData.FrontLedgeLocation + FVector(0.f, 0.f, ZOffset);
	FHitResult UpperPathHit = CapsuleTrace(FrontLedgePlusOffset, UpperSurfacePlusOffset, CapsuleRadius, CapsuleHalfHeight, true);
	
	// 3.1.Return if any obstacle is detected.
	if (UpperPathHit.bBlockingHit || UpperPathHit.bStartPenetrating)
	{
		UE_LOG(LogTemp, Warning, TEXT("Obstacle hasn't UpperSurface."));
		return false;
	}
	
	// 3.2.Update UpperSurface Data if any obstacle is not detected.
	EnvData.Obstacle_Data.bHasUpperSurface = true;
	EnvData.Obstacle_Data.UpperSurfaceLocation = UpperSurfaceLocation;
	DrawSphereTrace(UpperSurfaceLocation, 5.f, 5.0f);

	
	// 4.Update FrontHeight Value
	float FrontHeight = ObsData.FrontLedgeLocation.Z - (ActorLocation.Z - CapsuleHalfHeight);
	EnvData.Obstacle_Data.FrontHeight = FrontHeight;
	UE_LOG(LogTemp, Warning, TEXT("Obstacle Height : %f"), FrontHeight);

	if (!ObsData.bHasBackLedge)
	{
		// Do not need anymore data.
		return true;
	}
	else
	{
		DrawSphereTrace(ObsData.BackLedgeLocation, 5.f, 5.0f);
	}

	// 5.Update Depth Value (+Check if space from the FrontLedge to the BackLedge is clear)
	FVector BackLedgePlusOffset = ObsData.BackLedgeLocation + FVector(0.f, 0.f, ZOffset);
	FHitResult DepthPathHit = CapsuleTrace(FrontLedgePlusOffset, BackLedgePlusOffset, CapsuleRadius, CapsuleHalfHeight, false);
	float Depth;
	
	if (DepthPathHit.bBlockingHit) // 5.1.Hit : Depth is FrontLedge to HitObject Length.
	{
		Depth = GetDistance(ObsData.FrontLedgeLocation, DepthPathHit.ImpactPoint, ObsData.FrontLedgeNormal);
	}
	else // 5.2.Miss : Depth is FrontLedge to BackLedge Length.
	{
		Depth = GetDistance(ObsData.FrontLedgeLocation, ObsData.BackLedgeLocation, ObsData.FrontLedgeNormal);
	}

	// 5.3.Compare Depth Length To MinDepth (MinDepth is Length to the FrontLedge to UpperSurface) 
	float MinDepth = CapsuleRadius + 5.f;
	if (Depth < MinDepth) 
	{
		EnvData.Obstacle_Data.bHasUpperSurface = false;
		UE_LOG(LogTemp, Warning, TEXT("Obstacle Depth is Short."));
	}

	if (Depth > MaxObstacleDepth)
	{
		UE_LOG(LogTemp, Warning, TEXT("Obstacle Depth is Long."));
		return true;
	}

	EnvData.Obstacle_Data.Depth = Depth;
	UE_LOG(LogTemp, Warning, TEXT("Obstacle Depth : %f"), Depth);


	// 6.Update LandingSurface Data (+Check if space to the Landing surface is clear)
	// + Update BackDropHeight Value
	float DistanceOffset = CapsuleRadius + 10.f;
	FVector BackLedgePlusDistanceOffset = ObsData.BackLedgeLocation + (ObsData.BackLedgeNormal * DistanceOffset);
	float MaxDropDistance = FrontHeight + 50.f;
	FVector BackLedgePlusLandingOffset = ObsData.BackLedgeLocation + FVector(0.f, 0.f, -MaxDropDistance);
	FHitResult DropPathHit = CapsuleTrace(BackLedgePlusDistanceOffset, BackLedgePlusLandingOffset, CapsuleRadius, CapsuleHalfHeight, true);
	if (!DropPathHit.bBlockingHit)
	{
		return true;
	}
	float DropHeight = FMath::Abs(BackLedgePlusDistanceOffset.Z - DropPathHit.ImpactPoint.Z);
	float MinDropHeight = 20.f;
	if (DropHeight < MinDropHeight)
	{
		return true;
	}

	EnvData.Obstacle_Data.bHasLandingSurface = true;
	EnvData.Obstacle_Data.BackDropHeight = DropHeight;

	return true;
}

bool UParkourComponent::UpdateStepBoxData(FVector EdgeLocation, float Radius, FEnvData& EnvData, FVector ActorForward)
{
	/* return false mean "can't parkour, just jump." */

	/* ----- [Update List] ----- */
	/* FStepBoxData : LandingSurface Data */
	/* FStepBoxData : StepBox Value */

	// 1.Update NextStepBox Data
	FVector Start = EdgeLocation - FVector(0.f, 0.f, Radius) + (ActorForward * Radius);
	FHitResult NextStepBoxHitResult = LineTrace(Start, Start + (ActorForward * MaxStepBoxGapDistance), true);
	AParkourBlock* Block = Cast<AParkourBlock>(NextStepBoxHitResult.GetActor());
	if (Block == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("NextStepBox isn't ParkourBlock"));
		return false;
	}

	EnvData.StepBox_Data = Block->UpdateStepBoxData(NextStepBoxHitResult.ImpactPoint);
	FStepBoxData& BoxData = EnvData.StepBox_Data; // notice::It has just NextStepBox Ledge Transform.
	EnvData.HitComponent = NextStepBoxHitResult.GetComponent();

	if (!BoxData.bHasNextFrontLedge)
	{
		UE_LOG(LogTemp, Warning, TEXT("NextStepBox hasn't FrontLedge"));
		return false;
	}

	// 2.Update GapDepth
	float Gap = GetDistance(EdgeLocation, BoxData.NextFrontLedgeLocation, BoxData.NextFrontLedgeNormal);
	EnvData.StepBox_Data.GapDepth = Gap;
	UE_LOG(LogTemp, Warning, TEXT("Step Box Gap : %f"), Gap);


	// 3.Update LandingSurface Data
	float ZOffset = 5.f + CapsuleHalfHeight;
	FVector LandingLocation = BoxData.NextFrontLedgeLocation + (-BoxData.NextFrontLedgeNormal * (CapsuleRadius + 5.f));
	FVector LandingPlusOffset = LandingLocation + FVector(0.f, 0.f, ZOffset);
	FVector FrontLedgePlusOffset = BoxData.NextFrontLedgeLocation + FVector(0.f, 0.f, ZOffset);
	FHitResult LandingPathHit = CapsuleTrace(FrontLedgePlusOffset, LandingPlusOffset, CapsuleRadius, CapsuleHalfHeight, true);

	// 3.1.Return if any obstacle is detected.
	if (LandingPathHit.bBlockingHit || LandingPathHit.bStartPenetrating)
	{
		UE_LOG(LogTemp, Warning, TEXT("StepBox hasn't LandingSurface."));
		return false;
	}

	// 3.2.Check if there is a floor & Update LandingSurface Data if any obstacle is not detected.
	FVector LandingMinusOffset = LandingLocation - FVector(0.f, 0.f, ZOffset + 50.f);
	FHitResult FloorHit = SphereTrace(LandingPlusOffset, LandingMinusOffset, CapsuleRadius / 2.0f, true);
	if (!FloorHit.bBlockingHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("StepBox hasn't LandingSurface.")); 
		return false;
	}

	EnvData.StepBox_Data.bHasLandingSurface = true;
	EnvData.StepBox_Data.LandingSurfaceLocation = FVector(LandingLocation.X, LandingLocation.Y, FloorHit.ImpactPoint.Z);
	DrawSphereTrace(EnvData.StepBox_Data.LandingSurfaceLocation, 5.f, 5.0f);
	
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

float UParkourComponent::GetDistance(const FVector& StartLocation, const FVector& EndLocation, const FVector& FrontNormal)
{
	// 1. Z축을 무시한 Normal 구하기
	FVector OppositeNormal = -FrontNormal;
	OppositeNormal.Z = 0.f;
	OppositeNormal.Normalize();

	// 2. 내적(Dot Product)으로 길이 추출
	FVector DirectionVector = EndLocation - StartLocation;
	float Depth = FVector::DotProduct(DirectionVector, OppositeNormal);

	return FMath::Max(0.f, Depth);
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


FHitResult UParkourComponent::BoxTrace(const FVector& Start, const FVector& End, FVector BoxHalfSize, FRotator Rotation, bool bDrawDebug) const
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

FHitResult UParkourComponent::LineTrace(const FVector& Start, const FVector& End, bool bDrawDebug) const
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

FHitResult UParkourComponent::CapsuleTrace(const FVector& Start, const FVector& End, float Radius, float HalfHeight, bool bDrawDebug) const
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

void UParkourComponent::DrawSphereTrace(const FVector& Center, float Radius, float LifeTime) const
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

void FTraversalChooserParams::UpdateTraversalChooserParams(const FEnvData& CheckResult, ACharacter* Player)
{
	if (!Player) return;

	// 1.Update Player Movement Info
	Speed = Player->GetVelocity().Size2D();

	// 2.Update Obstacle Info
	ObstacleHeight = CheckResult.Obstacle_Data.FrontHeight;
	ObstacleDepth = CheckResult.Obstacle_Data.Depth;

	// 3.Update Step Box Info
	GapDepth = CheckResult.StepBox_Data.GapDepth;
}
