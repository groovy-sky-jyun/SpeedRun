// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourComponent.h"
#include "SpeedRunCharacter.h"
#include "ParkourMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "MotionWarpingComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameplayTagContainer.h"
#include "DA_AnimOption.h"
#include "DA_EnvironmentTags.h"
#include "DA_ParkourActionCategory.h"
#include "DA_JumpAction.h"
#include "DA_SphereTracesOption.h"
#include "DA_BoxTraceOption.h" 
#include "DA_LineTraceOption.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "ParkourBlock.h"
#include "Chooser.h"
#include "ChooserFunctionLibrary.h"    
#include "IObjectChooser.h" 
#include "InstancedStruct.h"



#define CHECK_VALID(Ptr) if(!Ptr) { UE_LOG(LogTemp, Error, TEXT("%s is Null!"), TEXT(#Ptr)); return; }
#define CHECK_ARRAY(Array) if(Array.IsEmpty()) { UE_LOG(LogTemp, Error, TEXT("%s is Empty!"), TEXT(#Array)); return; }

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
bool UParkourComponent::TryTraversalJumpAction()
{
	FVector ActorLocation = Player->GetActorLocation();
	FVector ActorForward = Player->GetActorForwardVector();
	float CapsuleRadius = Player->GetCapsuleComponent()->GetScaledCapsuleRadius();
	float CapsuleHalfHeight = Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FTraversalCheckResult TraversalResult = {};

	// 정면 장애물 감지
	AParkourBlock* Block;
	FHitResult ObstacleHitResult = TryDetectObstacle();

	if (!ObstacleHitResult.bBlockingHit)
	{
		// 일반 점프 | StepBoxJump
		UE_LOG(LogTemp, Warning, TEXT("NON Detect Obstacle. Start Jump or Step Box Jump"));
		UE_LOG(LogTemp, Warning, TEXT("Has Front Ledge : %f, Upper Surface : %f"), (float)TraversalResult.Obstacle_Data.bHasFrontLedge, (float)TraversalResult.Obstacle_Data.bHasUpperSurface);
		TryParkourChooser(TraversalResult);
		return true;
	}

	// 앞에 물체가 있는데 Traversal이 가능하지 않은 경우 -> 일반 점프
	Block = Cast<AParkourBlock>(ObstacleHitResult.GetActor());
	if (Block == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Detect Obstacle. But It's not ParkourBlock"));
		return true;
	}

	// Traversal이 가능한 경우 -> Obstacle 정보 담기
	TraversalResult = Block->GetLedgeTransform(ObstacleHitResult.ImpactPoint, ActorLocation);
	TraversalResult.HitComponent = ObstacleHitResult.GetComponent();

	FVector FrontLedgeLocation = TraversalResult.Obstacle_Data.FrontLedgeLocation;
	FVector BackLedgeLocation = TraversalResult.Obstacle_Data.BackLedgeLocation;

	if (TraversalResult.Obstacle_Data.bHasFrontLedge)
	{
		DrawSphereTrace(FrontLedgeLocation, 10.f, 5.0f);
	}
	else
	{
		// 물체가 감지 허용 범위 벗어난 경우 또는 traversal 조건에 부합하지 않은 경우
		UE_LOG(LogTemp, Warning, TEXT("Detect Obstacle. But Can't Parkour."));
		return true;
	}

	if (TraversalResult.Obstacle_Data.bHasBackLedge)
	{
		DrawSphereTrace(BackLedgeLocation, 10.f, 5.0f);
	}

	// 점프하는 궤도에 장애물 없는지 체크 -> 없다면 일반 Jump
	FVector FrontLedgeNormal = TraversalResult.Obstacle_Data.FrontLedgeNormal;
	
	FVector FrontLedgeTopSurfaceLocation = FrontLedgeLocation + FVector(0.f, 0.f, CapsuleHalfHeight + 2.f) + (FrontLedgeNormal * (CapsuleRadius + 2.0));

	float StartZOffset = FrontLedgeTopSurfaceLocation.Z - 70.f;
	FVector Start = FVector(ActorLocation.X, ActorLocation.Y, StartZOffset);

	FHitResult FrontLedgeRoomHitResult = CapsuleTrace(Start, FrontLedgeTopSurfaceLocation, CapsuleRadius/2, CapsuleHalfHeight, true);
	if (FrontLedgeRoomHitResult.bBlockingHit | FrontLedgeRoomHitResult.bStartPenetrating)
	{
		TraversalResult.Obstacle_Data.bHasFrontLedge = false;
		UE_LOG(LogTemp, Warning, TEXT("Detect Obstacle. But It hasn't Surface."));
		return true;
	}

	// 물체 높이 체크
	float ObstacleHeight = FMath::Abs((ActorLocation.Z - CapsuleHalfHeight) - FrontLedgeLocation.Z);
	TraversalResult.Obstacle_Data.ObstacleHeight = ObstacleHeight;
	UE_LOG(LogTemp, Warning, TEXT("Obstacle Height : %f"), ObstacleHeight);

	// 물체 Depth 체크
	FVector BackLedgeNormal = TraversalResult.Obstacle_Data.BackLedgeNormal;
	FVector BackLedgeTopSurfaceLocation = BackLedgeLocation + FVector(0.f, 0.f, CapsuleHalfHeight + 2.f) + (BackLedgeNormal * (CapsuleRadius + 2.0));
	FHitResult TopSweepResult = CapsuleTrace(FrontLedgeTopSurfaceLocation, BackLedgeTopSurfaceLocation, CapsuleRadius, CapsuleHalfHeight, false);
	if (!TopSweepResult.bBlockingHit) //BackLedge까지를 Depth로 지정
	{
		FVector ObstacleDepthVector = FrontLedgeLocation - BackLedgeLocation;
		float ObstacleDepth = ObstacleDepthVector.Size2D();
		TraversalResult.Obstacle_Data.ObstacleDepth = ObstacleDepth;
		UE_LOG(LogTemp, Warning, TEXT("Obstacle Depth : %f"), ObstacleDepth);

		// 착지 지점과의 높이차 데이터 구하기
		FVector FloorFrontLocation = BackLedgeLocation + (BackLedgeNormal * (CapsuleRadius + 2.0));
		float FloorZOffset = ObstacleHeight + 50.f;
		FVector EndLocation = FloorFrontLocation - FVector(0.f, 0.f, FloorZOffset) + FVector(0.f, 0.f, CapsuleHalfHeight); //플레이어 중심이 바닥에 위치하면 안되므로 CapsuleHalfHeight를 더해준다.
		FHitResult SurfaceHitResult = CapsuleTrace(BackLedgeTopSurfaceLocation, EndLocation, CapsuleRadius, CapsuleHalfHeight, false);

		if (SurfaceHitResult.bBlockingHit && !SurfaceHitResult.bStartPenetrating)
		{
			TraversalResult.Obstacle_Data.bHasBackFloor = true;
			TraversalResult.Obstacle_Data.BackFloorLocation = SurfaceHitResult.ImpactPoint;
			TraversalResult.Obstacle_Data.BackLedgeHeight = FMath::Abs(SurfaceHitResult.ImpactPoint.Z - BackLedgeLocation.Z);
			UE_LOG(LogTemp, Warning, TEXT("Obstacle Back Ledge Height : %f"), TraversalResult.Obstacle_Data.BackLedgeHeight);
		}
		else
		{
			TraversalResult.Obstacle_Data.bHasBackFloor = false;
		}
	}
	else //충돌 지점 앞까지를 Depth로 지정
	{
		FVector ObstacleDepthVector = TopSweepResult.ImpactPoint - FrontLedgeLocation;
		float ObstacleDepth = ObstacleDepthVector.Size2D();
		TraversalResult.Obstacle_Data.ObstacleDepth = ObstacleDepth;
		TraversalResult.Obstacle_Data.bHasBackLedge = false;
		UE_LOG(LogTemp, Warning, TEXT("Obstacle Depth : %f"), ObstacleDepth);
	}

	TryParkourChooser(TraversalResult);
	return true;
}

UAnimMontage* UParkourComponent::TryParkourChooser(FTraversalCheckResult& CheckResult)
{
	if (!CHT_TraversalAnims || !Player)
	{
		return nullptr;
	}

	FTraversalChooserParams ChooserParams = {};
	ChooserParams.InitializeFromContext(CheckResult, Player);

	FChooserEvaluationContext Context;
	Context.AddStructParam(ChooserParams);
	Context.AddObjectParam(Player);

	FInstancedStruct ChooserStruct = UChooserFunctionLibrary::MakeEvaluateChooser(CHT_TraversalAnims);
	UObject* Result = UChooserFunctionLibrary::EvaluateObjectChooserBase(
		Context,
		ChooserStruct,
		UAnimMontage::StaticClass()
	);

	UAnimMontage* AnimMontage = Cast<UAnimMontage>(Result);
	if (AnimMontage == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Action AnimMontage is null"));
		return nullptr;
	}

	return AnimMontage;
}

void UParkourComponent::PlayAminMontage(const FTraversalCheckResult& TraversalResult) const
{
	if (!TraversalResult.ChosenMontage) return;

	AnimInstance->Montage_Play(TraversalResult.ChosenMontage, TraversalResult.PlayRate);
}

void UParkourComponent::DoLanding()
{
	/*
	FVector FootLocation = Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

	FHitResult SurfaceHitResult = ScanSurfaceEdge(ETraceDirection::Vertical, FootLocation, Player->GetActorForwardVector(), true, true);

	if (SurfaceHitResult.bBlockingHit)
	{
		FHitResult HitResult = BoxTrace(SurfaceHitResult.ImpactPoint, Player->GetActorForwardVector(), Player->GetActorRotation(), true);
		
		bCanLanding = true;
	}*/
	return;
}





//=================================
//           환경 감지 
//=================================
FHitResult UParkourComponent::TryDetectObstacle()
{
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Player);

	FVector Start = Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 40.f);
	FVector End = Start + Player->GetActorForwardVector() * 220.f;
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_GameTraceChannel1, Params);
	DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 3.f);

	return HitResult;
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

void FTraversalChooserParams::InitializeFromContext(const FTraversalCheckResult& CheckResult, ACharacter* Player)
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
	bHasLandingSurface = CheckResult.StepBox_Data.bHasLandingSurface;
	GapDepth = CheckResult.StepBox_Data.GapDepth;
}
