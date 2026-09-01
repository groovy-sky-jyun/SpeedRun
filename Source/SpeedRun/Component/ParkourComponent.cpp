// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourComponent.h"
#include "SpeedRunCharacter.h"
#include "ParkourMovementComponent.h"
#include "MotionWarpingComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ParkourBlock.h"
#include "ParkourActionBase.h"
#include "Animation/AnimInstance.h"

UParkourComponent::UParkourComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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


//=================================
//      액션 수행 (Execution)         
//=================================
void UParkourComponent::PerformJumpSequence()
{
	// EnvData Reset
	FEnvData EnvData = {};

	// 1.Update EnvData & Check Can Parkour
	bool bCanParkour = TryUpdateEnvData(EnvData);
	if (!bCanParkour)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't Parkour Jump"));
		Player->Jump();
		return;
	}

	// 2.Evaluate ActionType by EnvData
	EnvData.CurrentSpeed = Player->GetVelocity().Size2D();
	CurrentEnvData = EnvData;
	
	UParkourActionBase* BestAction = EvaluateNextAction(EnvData);

	if (BestAction)
	{
		UE_LOG(LogTemp, Log, TEXT("Selected Parkour Action: %s"), *BestAction->GetName());
		BestAction->ExecuteAction(this, EnvData, Player);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("No Valid Parkour Action Found."));
		Player->Jump();
	}	
}

UParkourActionBase* UParkourComponent::EvaluateNextAction(const FEnvData& EnvData)
{
	UParkourActionBase* BestAction = nullptr;
	float HighestScore = -1.0f;

	for (UParkourActionBase* Action : RegisteredActions)
	{
		if (Action)
		{
			float Score = Action->Evaluate(this, EnvData, Player);
			if (Score > HighestScore)
			{
				HighestScore = Score;
				BestAction = Action;
			}
		}
	}

	return BestAction;
}


//=================================
//      Hang 및 Drop         
//=================================
bool UParkourComponent::IsHanging() const
{
	if (!ParkourMovement) return false;

	return ParkourMovement->MovementMode == MOVE_Custom
		&& ParkourMovement->CustomMovementMode == static_cast<uint8>(ECustomMovementMode::CUSTOM_Hang);
}

void UParkourComponent::DropFromHang()
{
	if (!IsHanging()) return;

	if (AnimInstance && AnimInstance->IsAnyMontagePlaying())
	{
		AnimInstance->StopAllMontages(0.2f);
	}

	ParkourMovement->SetMovementMode(MOVE_Falling);
	ParkourMovement->CustomMovementMode = static_cast<uint8>(ECustomMovementMode::CUSTOM_None);
	Player->GetCharacterMovement()->Velocity = FVector::ZeroVector;
	ParkourMovement->bOrientRotationToMovement = true;
}

void UParkourComponent::EnterHangState()
{
	if (IsHanging()) return;

	Player->GetCharacterMovement()->StopMovementImmediately();
	Player->GetCharacterMovement()->SetMovementMode(MOVE_Custom, static_cast<uint8>(ECustomMovementMode::CUSTOM_Hang));

	// Block the Rotate the Character toward the direction of acceleration 
	Player->GetCharacterMovement()->bOrientRotationToMovement = false;
}


// Hang 실행 전 방향 벽으로 맞춰주고, 위치 벽에서 조금 떨어진 위치로 맞춰줌
void UParkourComponent::AlignToLedge(const FEnvData& EnvData)
{
	const FObstacleData& ObsData = EnvData.Obstacle_Data;

	FRotator TargetRotation = (-ObsData.FrontLedgeNormal).Rotation();
	TargetRotation.Pitch = 0.f;
	TargetRotation.Roll = 0.f;

	FVector TargetLocation = ObsData.FrontLedgeLocation + (ObsData.FrontLedgeNormal * (CapsuleRadius + 80.f));
	TargetLocation.Z -= CapsuleHalfHeight * 2 + 10.f;

	Player->SetActorLocationAndRotation(TargetLocation, TargetRotation);
}


//=================================
//      Motion Warping       
//=================================
void UParkourComponent::AddWarpTarget(FName TargetName, FVector Location, FVector Normal)
{
	if (!WarpComponent) return;

	FRotator TargetRotation = (-Normal).Rotation();
	// Yaw 만 적용
	TargetRotation.Pitch = 0.f;
	TargetRotation.Roll = 0.f;

	WarpComponent->AddOrUpdateWarpTargetFromLocationAndRotation(TargetName, Location, TargetRotation);
}

void UParkourComponent::ClearAllWarpTargets()
{
	if (!WarpComponent) return;

	WarpComponent->RemoveAllWarpTargets();
}


//=================================
//      Detect Environment
//=================================
bool UParkourComponent::TryUpdateEnvData(FEnvData& EnvData)
{
	FVector ActorLocation = Player->GetActorLocation();
	FVector ActorForward = Player->GetActorForwardVector();
	
	FHitResult ObstacleHitResult = TryDetectObstacle(ActorLocation, ActorForward);

	// 1.not found Obstacle
	if (!ObstacleHitResult.bBlockingHit)
	{
		return false;
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

FHitResult UParkourComponent::TryDetectObstacle(FVector ActorLocation, FVector ActorForward)
{
	FHitResult HitResult;
	float Radius = 150.f;

	// 1.컨트롤러가 바라보는 방향 가져오기
	FVector CameraForward = Player->GetControlRotation().Vector();

	// 2.ParkourBlock 감지 Trace 생성
	FVector Start = Player->GetActorLocation() + FVector(0.f, 0.f, 50.f) + (CameraForward * Radius / 2);
	FVector End = Start + (CameraForward * 300.f);

	HitResult = SphereTrace(Start, End, Radius, ECollisionChannel::ECC_GameTraceChannel1, bShowDebugTrace);

	return HitResult;
}

bool UParkourComponent::UpdateObstacleData(FHitResult ObstacleHitResult, AParkourBlock* Block, FEnvData& EnvData, FVector ActorLocation)
{
	/* return false mean "can't parkour, just jump." */

	// 1.Update ParkourTag, Front/BackLedge Transform by ParkourBlock
	EnvData.HitParkourTag = Block->GetParkourTag();
	EnvData.Obstacle_Data = Block->UpdateObstacleData(Player, ObstacleHitResult.ImpactPoint, Player->GetActorLocation());
	EnvData.HitComponent = ObstacleHitResult.GetComponent();
	FObstacleData& ObsData = EnvData.Obstacle_Data; // notice::It has just Front/Back Ledge Transform.


	// 2.Check It has FrontLedge
	if (!ObsData.bHasFrontLedge)
	{
		return false;
	}

	// 3.Update UpperSurface Data (+Check if space to the upper surface is clear)
	float ZOffset = 5.f + CapsuleHalfHeight;
	FVector UpperSurfaceLocation = ObsData.FrontLedgeLocation + (-ObsData.FrontLedgeNormal * 20.f);
	FVector UpperSurfacePlusOffset = UpperSurfaceLocation + FVector(0.f, 0.f, ZOffset);
	FVector FrontLedgePlusOffset = ObsData.FrontLedgeLocation + FVector(0.f, 0.f, ZOffset);
	
	FHitResult UpperPathHit = CapsuleTrace(FrontLedgePlusOffset, UpperSurfacePlusOffset, CapsuleRadius, CapsuleHalfHeight, bShowDebugTrace);

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

	EnvData.Obstacle_Data.Depth = Depth;
	UE_LOG(LogTemp, Warning, TEXT("Obstacle Depth : %f"), Depth);


	// 6.Update LandingSurface Data (+Check if space to the Landing surface is clear)
	// + Update BackDropHeight Value
	FVector LandingLocation = FVector();
	float DropHeight = 0.f;
	if (CanLanding(EnvData, LandingLocation, DropHeight))
	{
		EnvData.Obstacle_Data.bHasLandingSurface = true;
		EnvData.Obstacle_Data.LandingSurfaceLocation = LandingLocation;
		EnvData.Obstacle_Data.BackDropHeight = DropHeight;
	}

	return true;
}

bool UParkourComponent::CanLanding(const FEnvData& EnvData, FVector& LandingLocation, float& DropHeight)
{
	//1. Edge 모서리 뒷편에 바닥이 있는지 확인
	const FObstacleData& ObsData = EnvData.Obstacle_Data;
	FVector Start = ObsData.BackLedgeLocation + (ObsData.BackLedgeNormal * (CapsuleRadius + 20.f));
	float Distance = ObsData.BackLedgeLocation.Z + 50.f; // 최대 감지 길이
	FHitResult Hit;

	for (int i = 0; i < 3; i++)
	{
		FVector StartLocation = Start + (ObsData.BackLedgeNormal * (CapsuleRadius * 2 * i));
		FVector EndLocation = StartLocation + FVector(0.f, 0.f, -Distance);

		Hit = SphereTrace(StartLocation, EndLocation, CapsuleRadius, ECollisionChannel::ECC_Visibility, false);

		if (!Hit.bBlockingHit)
		{
			return false;
		}
	}

	//2. 바닥이 있다면 BackLedge ~ 바닥까지 높이 확인
	float Height = FMath::Abs(ObsData.BackLedgeLocation.Z - Hit.ImpactPoint.Z);
	if (Height > MinHeightBlock)
	{
		LandingLocation = Hit.ImpactPoint;
		DropHeight = Height;
		return true;
	}

	return false;
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
//       Basic Trace Logic
//=================================
FHitResult UParkourComponent::SphereTrace(const FVector& Start, const FVector& End, float Radius, ECollisionChannel TraceChannel, bool bDrawDebug) const
{
	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Player);

	UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(),
		Start,
		End,
		Radius,
		UEngineTypes::ConvertToTraceType(TraceChannel), // Trace Channel
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
	if (!bShowDebugTrace)
	{
		return;
	}

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
