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

EParkourActionType UParkourComponent::EvaluateNextAction(const FEnvData& InEnvData)
{
	//** 태그 기반 액션 판별 **//
	if (InEnvData.HitParkourTag.IsValid())
	{
		static const FGameplayTag Tag_Bar = FGameplayTag::RequestGameplayTag(FName("ParkourBlock.Precision.Bar"));
		static const FGameplayTag Tag_Pole = FGameplayTag::RequestGameplayTag(FName("ParkourBlock.Precision.Pole"));
		static const FGameplayTag Tag_WallLedge = FGameplayTag::RequestGameplayTag(FName("ParkourBlock.Precision.WallLedge"));
		FGameplayTag ParkourTag = InEnvData.HitParkourTag;

		if (ParkourTag.MatchesTag(Tag_Bar))
		{
			return EParkourActionType::PARKOUR_Swing;
		}
		else if (ParkourTag.MatchesTag(Tag_Pole))
		{
			return EParkourActionType::PARKOUR_Pole;
		}
		else if (ParkourTag.MatchesTag(Tag_WallLedge))
		{
			return EParkourActionType::PARKOUR_WallSidle;
		}
		return EParkourActionType::PARKOUR_None;
	}

	if (InEnvData.Obstacle_Data.FrontHeight < MinHeightBlock)
	{
		return EParkourActionType::PARKOUR_None;
	}

	EMovementMode MovementMode = Player->GetCharacterMovement()->MovementMode;
	uint8 CustomMode = Player->GetParkourMovement()->CustomMovementMode;

	if (CanVault(InEnvData, MovementMode)) return EParkourActionType::PARKOUR_Vault;
	if (CanMantle(InEnvData, MovementMode, CustomMode)) return EParkourActionType::PARKOUR_Mantle;
	if (CanHang(InEnvData, MovementMode)) return EParkourActionType::PARKOUR_Hang;
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

bool UParkourComponent::CanVault(const FEnvData& InEnvData, EMovementMode CurrentMode)
{
	if (CurrentMode != EMovementMode::MOVE_Walking) return false;

	const FObstacleData& ObsData = InEnvData.Obstacle_Data;

	if (ObsData.FrontHeight > MaxHeightVault) return false;
	if (ObsData.Depth > MaxDepthVault) return false;
	
	return true;
}

bool UParkourComponent::CanMantle(const FEnvData& InEnvData, EMovementMode CurrentMode, uint8 CustomMode)
{
	const FObstacleData& ObsData = InEnvData.Obstacle_Data;
	// CurrentMode / CustomMode
	if (CurrentMode == EMovementMode::MOVE_Custom && CustomMode == static_cast<uint8>(ECustomMovementMode::CUSTOM_Hang))
	{
		return ObsData.Depth >= CapsuleRadius * 2.f;
	}

	if (CurrentMode != EMovementMode::MOVE_Walking) return false;

	if (ObsData.FrontHeight <= MaxHeightVault)
	{
		if (ObsData.Depth > MaxDepthVault)
		{
			return true;
		}
	}

	if (ObsData.FrontHeight > MaxHeightVault && ObsData.FrontHeight <= MaxHeightMantle)
	{
		if (ObsData.Depth >= CapsuleRadius * 2.f)
		{
			return true;
		}
	}
	return false;
}

bool UParkourComponent::CanHang(const FEnvData& InEnvData, EMovementMode CurrentMode)
{
	const FObstacleData& ObsData = InEnvData.Obstacle_Data;

	if (CurrentMode == EMovementMode::MOVE_Falling)
	{
		return ObsData.bHasFrontLedge;
	}

	if (CurrentMode != EMovementMode::MOVE_Walking) return false;

	if (ObsData.FrontHeight > MaxHeightVault && ObsData.FrontHeight <= MaxHeightMantle)
	{
		if (ObsData.Depth < CapsuleRadius * 2.f)
		{
			return true;
		}
	}

	return false;
}


//=================================
//           환경 감지 
//=================================
FHitResult UParkourComponent::TryDetectObstacle(FVector ActorLocation, FVector ActorForward)
{
	FHitResult HitResult;
	float Radius = 150.f;

	// 1.컨트롤러가 바라보는 방향 가져오기
	FVector CameraForward = Player->GetControlRotation().Vector();

	// 2.ParkourBlock 감지 Trace 생성
	FVector Start = Player->GetActorLocation() + FVector(0.f, 0.f, 50.f) + (CameraForward * Radius/2);
	float TraceDistance = 300.f;
	FVector End = Start + (CameraForward * TraceDistance);

	HitResult = SphereTrace(Start, End, Radius, ECollisionChannel::ECC_GameTraceChannel1, true);


	return HitResult;
}

bool UParkourComponent::UpdateObstacleData(FHitResult ObstacleHitResult, AParkourBlock* Block, FEnvData& EnvData, FVector ActorLocation)
{
	/* return false mean "can't parkour, just jump." */

	/* ----- [Update List] ----- */
	/* FObstacleData : UpperSurface Data */
	/* FObstacleData : LandingSurface Data */
	/* FObstacleData : Obstacle Value */

	// 1.Update ParkourTag, Front/BackLedge Transform by ParkourBlock
	EnvData.HitParkourTag = Block->GetParkourTag();
	EnvData.Obstacle_Data = Block->UpdateObstacleData(Player, ObstacleHitResult.ImpactPoint, Player->GetActorLocation());
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
	FVector UpperSurfaceLocation = ObsData.FrontLedgeLocation + (-ObsData.FrontLedgeNormal * 20.f); 
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

bool UParkourComponent::CanLanding(const FEnvData& InEnvData, FVector& LandingLocation, float& DropHeight)
{
	//1. Edge 모서리 뒷편에 바닥이 있는지 확인
	const FObstacleData& ObsData = InEnvData.Obstacle_Data;
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
//      Basic Trace Logic
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
}
