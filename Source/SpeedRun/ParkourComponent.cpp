// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourComponent.h"
#include "SpeedRunCharacter.h"
#include "ParkourMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "MotionWarpingComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameplayTagContainer.h"
#include "DA_EnvironmentTags.h"
#include "DA_ParkourActionCategory.h"
#include "DA_JumpAction.h"
#include "DA_SphereTracesOption.h"
#include "DA_BoxTraceOption.h" 
#include "DA_LineTraceOption.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

UParkourComponent::UParkourComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UParkourComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<ASpeedRunCharacter>(GetOwner());

	if (Player)
	{
		AnimInstance = Player->GetMesh()->GetAnimInstance();
		WarpComponent = Player->GetMotionWarpingComponent();
		ParkourMovement = Player->GetParkourMovement();
	}
}


void UParkourComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

//=================================
//      액션 수행 (Execution)         
//=================================
void UParkourComponent::TryParkourAction()
{
	/*
	* 파쿠르 액션 AnimMontage + Motion Warping 실행 *
	*/
	/*
	if (!ActionData)
	{
		UE_LOG(LogTemp, Warning, TEXT("Parkour DataAsset is null"));
		return;
	}
	
	if (WarpComponent)
	{
		FVector NewLocation = Player->GetActorLocation();
		NewLocation = NewLocation + FVector(0.f, 0.f, MotionWarpingZOffset) + (Player->GetActorForwardVector() * MotionWarpingDistance);

		FName WarpTargetName = ActionData->WarpTargetName;
		WarpComponent->AddOrUpdateWarpTargetFromLocationAndRotation(WarpTargetName, NewLocation, Player->GetActorRotation());

		UE_LOG(LogTemp, Warning, TEXT("WarpTargetName : %s"), *WarpTargetName.ToString());
	}

	if (UAnimMontage* AnimMontage = ActionData->AnimMontage)
	{
		AnimInstance->Montage_Play(AnimMontage);
	}*/
}

void UParkourComponent::HandleToJump()
{
	// 태그 초기화
	CurrentEnvironmentTags.Reset();
	
	if (ParkourMovement->IsFalling())
	{
		DoLanding();
	}
	else 
	{
		UE_LOG(LogTemp, Warning, TEXT("======RESET======"));

		// 태그 초기화
		CurrentActionTags.Reset();

		// 장애물 감지 
		bool bObstacleHit = DetectObstacle();

		if (bObstacleHit)
		{
			MeasureObstacleDimensions();
		}
		else
		{
			AnalyzeEdgeEnvironment();
		}	

		AddActionTag(Tag_Jump);

		SetupJumpPhysics();

		Player->Jump();
	}
}

void UParkourComponent::DoLanding()
{
	/** 1.Detect Surface to Foot Under **/
	FVector FootLocation = Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FHitResult SurfaceHitResult = DetectSphereTraces(Tag_SurfaceSpace, FootLocation, Player->GetActorForwardVector(), true, ETraceDirection::Vertical, true);

	if (SurfaceHitResult.bBlockingHit)
	{
		/** 2.Landing Surface Space Check  **/
		FHitResult HitResult = BoxTrace(Tag_SurfaceSpace, SurfaceHitResult.ImpactPoint, Player->GetActorForwardVector(), Player->GetActorRotation(), true);

		UpdateEnvironmentTags(Tag_SurfaceSpace, HitResult.bBlockingHit);
	}

	/** 3.Add Landing Action Tag **/
	AddActionTag(Tag_Landing);
	
	/** 4.Execute Landing Amin Montage **/
	/**************수정필요*****************/
	//TryParkourAction();
}


void UParkourComponent::SetupJumpPhysics()
{
	// 1.Find JumpTag in CurrentActions
	FGameplayTag JumpTag = FindChildActionTag(Tag_Jump);

	// 2.Find JumpOption DA
	FJumpOption Option = FindJumpOption(JumpTag);

	if (Option.JumpTagName.IsValid() && ParkourMovement)
	{
		// 3.점프 설정
		ParkourMovement->SetJumpValues(Option.GravityScale, Option.ZVelocity, Option.Impulse);
	}
}


//=================================
//   Find DataAsset (feat.Tag)
//=================================
FGameplayTag UParkourComponent::FindChildActionTag(const FGameplayTag& ParentTag) const
{
	FGameplayTagContainer Filtered = CurrentActionTags.Filter(FGameplayTagContainer(ParentTag));

	return Filtered.First();
}

void UParkourComponent::UpdateEnvironmentTags(const FGameplayTag& TagCategory, float Value)
{
	if (DA_EnvironmentTags.IsEmpty()) return;

	for (const auto& DA : DA_EnvironmentTags)
	{
		// 1.유효성 확인
		if (!IsValid(DA) || !DA->CategoryTag.MatchesTag(TagCategory)) continue;

		// 2.조건 일치하는 태그 추가
		for (const FEnvironmentState& TagDetails : DA->TagList)
		{
			if (Value > TagDetails.MinValue && Value <= TagDetails.MaxValue)
			{
				CurrentEnvironmentTags.AddTag(TagDetails.Tag);
				UE_LOG(LogTemp, Warning, TEXT("[EnvironmentTag] : %s"), *TagDetails.Tag.ToString());
				return;
			}
		}
	}
}

void UParkourComponent::AddActionTag(const FGameplayTag& TagCategory)
{
	FGameplayTag NewActionTag = SelectActionTagOnContext(TagCategory);
	if (NewActionTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ActionTag] : %s"), *NewActionTag.ToString());

		/** 1.Add ActionTag **/
		CurrentActionTags.AddTag(NewActionTag);
	}
}

FGameplayTag UParkourComponent::SelectActionTagOnContext(const FGameplayTag& ActionCategory) const
{
	if (ActionCategoryList.IsEmpty()) return FGameplayTag::EmptyTag;

	FGameplayTagContainer HaveTags = CurrentActionTags;
	HaveTags.AppendTags(CurrentEnvironmentTags); 

	for (const auto& CategoryDA : ActionCategoryList)
	{
		// 1.유효성 확인
		if (!IsValid(CategoryDA) || !CategoryDA->ActionCategory.MatchesTag(ActionCategory)) continue;

		// 2.액션 카테고리와 일치하는 DA 찾기
		for (const auto& ActionEntry : CategoryDA->ActionList)
		{
			// 3. Action Condition과 CurrentTags가 일치하는 ActionTag 찾기

			for (const auto& ConditionContainer : ActionEntry.ConditionTags)
			{
				if (HaveTags.HasAll(ConditionContainer))
				{
					return ActionEntry.ActionTag;
				}
			}
		}
	}

	return FGameplayTag::EmptyTag;
}

FJumpOption UParkourComponent::FindJumpOption(const FGameplayTag& NewTag) const
{
	if (!JumpConfigs) return FJumpOption();

	for (const FJumpOption& List : JumpConfigs->JumpList)
	{
		if (NewTag.MatchesTag(List.JumpTagName))
		{
			return List;
		}
	}

	return FJumpOption();
}

//=================================
// 장애물 및 환경 감지 (Trace Logic)
//=================================
bool UParkourComponent::DetectObstacle()
{
	// Detect FrontObstacle (LineTrace_Horizontal)
	FVector Start = Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FHitResult HitResult = LineTraceHor(Tag_Detect, Start, Player->GetActorForwardVector(), false);

	// 2.Detect 환경 태그 추가
	FGameplayTag NewDetectTag;
	if (HitResult.bBlockingHit)
	{
		NewDetectTag = Tag_DetectObstacle;
	}
	else
	{
		NewDetectTag = Tag_DetectNone;
	}

	UE_LOG(LogTemp, Warning, TEXT("[EnvironmentTag] : %s"), *NewDetectTag.ToString());
	CurrentEnvironmentTags.AddTag(NewDetectTag);

	return HitResult.bBlockingHit;
}

void UParkourComponent::MeasureObstacleDimensions()
{
	/** 1.Add Obstacle Height (Horizontal Sphere Traces) */
	FVector ObstacleHeightStart = Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FHitResult HeightHitResult = DetectSphereTraces(Tag_ObstacleHeight, ObstacleHeightStart, Player->GetActorForwardVector(), false, ETraceDirection::Horizontal, true);

	if (HeightHitResult.bBlockingHit)
	{
		UpdateEnvironmentTags(Tag_ObstacleHeight, HeightHitResult.ImpactPoint.Z);
	}

	/** 2.Add Obstacle Width (Vertical Sphere Traces) */
	FVector ObstacleWidthStart = HeightHitResult.ImpactPoint;
	FHitResult WidthHitResult = DetectSphereTraces(Tag_ObstacleWidth, ObstacleWidthStart, Player->GetActorForwardVector(), false, ETraceDirection::Vertical, true);
	float ObstacleWidth = FVector::Dist(ObstacleWidthStart, WidthHitResult.ImpactPoint);

	//UE_LOG(LogTemp, Warning, TEXT("Obstacle width is : %f"), ObstacleWidth);
	if (WidthHitResult.bBlockingHit)
	{
		UpdateEnvironmentTags(Tag_ObstacleWidth, ObstacleWidth);
	}
}

void UParkourComponent::AnalyzeEdgeEnvironment()
{
	/** 1. Check Edge (Short Vertical Sphere Traces) **/
	FVector PlayerFootLocation = Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FHitResult EdgeHitResult = DetectSphereTraces(Tag_DetectNone, PlayerFootLocation, Player->GetActorForwardVector(), false, ETraceDirection::Vertical, false);

	/** 2. Check 'Step Box' or 'Rooftop'? (Long Vertical Line Trace) **/
	if (EdgeHitResult.bBlockingHit)
	{
		FVector EdgeStart = EdgeHitResult.ImpactPoint;
		FHitResult EdgeHeightHitResult = LineTraceVer(Tag_DetectNone, EdgeStart, Player->GetActorForwardVector(), false);

		// [Step Box]
		if (EdgeHeightHitResult.bBlockingHit)
		{
			MeasureStepBoxWidth(EdgeHeightHitResult.ImpactPoint);
			
		}
		else // [Rooftop]
		{
			MeasureBuildingGap(PlayerFootLocation);
		}
	}
}

void UParkourComponent::MeasureStepBoxWidth(const FVector& StepOverStart)
{
	/** 3.Add StepBox Width (Long Horizontal Line Trace) **/
	FHitResult StepWidthHitResult = LineTraceHor(Tag_StepBoxGap, StepOverStart, Player->GetActorForwardVector(), false);

	if (StepWidthHitResult.bBlockingHit)
	{
		UpdateEnvironmentTags(Tag_StepBoxGap, FVector::Dist(StepOverStart, StepWidthHitResult.ImpactPoint));
	}
}

void UParkourComponent::MeasureBuildingGap(const FVector& PlayerFootLocation)
{
	/** 3.Get Horizontal Building Gap (Long Horizontal Sphere Traces) **/
	FVector BuildingEdgeStart = Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FHitResult BuildingHorHitResult = DetectSphereTraces(Tag_BuildingGap, PlayerFootLocation, Player->GetActorForwardVector(), true, ETraceDirection::Horizontal, false);

	if (BuildingHorHitResult.bBlockingHit)
	{
		/** 4.Get Vertical Building Height (Long Vertical Line Trace) **/
		FVector BuildingHeightStart = BuildingHorHitResult.ImpactPoint;
		FHitResult BuildingVerHitResult = LineTraceVer(Tag_BuildingGap, BuildingHeightStart, -BuildingHorHitResult.ImpactNormal, false);

		/** 5.Add Surface Gap (The distance from Rooftop A to Rooftop B) **/
		if (BuildingVerHitResult.bBlockingHit)
		{
			float BuildingGap = FVector::Dist(Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), BuildingVerHitResult.ImpactPoint);
			UpdateEnvironmentTags(Tag_BuildingGap, BuildingGap);
		}
	}
}


//=================================
//      Basic Trace Logic
//=================================
const UDA_SphereTracesOption* UParkourComponent::GetTraceDA_Spheres(FGameplayTag TagCategory) const
{
	return GetTraceDA(SphereTracesOptions, TagCategory);
}

const UDA_BoxTraceOption* UParkourComponent::GetTraceDA_Box(FGameplayTag TagCategory) const
{
	return GetTraceDA(BoxTraceOptions, TagCategory);
}

const UDA_TraceOptions* UParkourComponent::GetTraceDA_Line(FGameplayTag TagCategory) const
{
	return GetTraceDA(LineTraceOptions, TagCategory);
}


FHitResult UParkourComponent::DetectSphereTraces(FGameplayTag TagCategory, FVector Start, FVector Dir, bool bReturnHit, ETraceDirection TraceDir, bool bDrawDebug) const
{
	const UDA_SphereTracesOption* Option = GetTraceDA_Spheres(TagCategory);

	if (!Option) return FHitResult();

	FHitResult LastHitResult;

	for (int i = 0; i < Option->Count; i++)
	{
		FVector StartLocation, EndLocation;

		if (TraceDir == ETraceDirection::Horizontal)
		{
			StartLocation = Start + (Dir * Option->FrontOffset) + FVector(0.f, 0.f, Option->ZOffset + (Option->Gap * i));
			EndLocation = StartLocation + Dir * Option->Distance;
		}
		else
		{

			StartLocation = Start + FVector(0.f, 0.f, Option->ZOffset) + (Dir * (Option->FrontOffset + (Option->Gap * i)));
			EndLocation = StartLocation + FVector(0.f, 0.f, Option->Distance);
		}

		FHitResult HitResult = SphereTrace(StartLocation, EndLocation, Option->Radius, bDrawDebug);

		if (bReturnHit) //immediately return if find surface to floor.
		{
			if (HitResult.bBlockingHit) return HitResult;
		}
		else { //낭떨어지 직전의 바닥 위치 반환 (낭떨어지 감지)
			if (HitResult.bBlockingHit)
			{
				if (i == Option->Count - 1) //낭떨어지를 감지하지 못한 경우
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


FHitResult UParkourComponent::BoxTrace(FGameplayTag TagCategory, FVector Start, FVector Dir, FRotator Rotation, bool bDrawDebug) const
{
	const UDA_BoxTraceOption* Option = GetTraceDA_Box(TagCategory);

	if (!Option) return FHitResult();

	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Player);

	FVector StartLocation = Start + FVector(0.f, 0.f, Option->ZOffset) + (Dir * Option->FrontOffset);
	FVector EndLocation = StartLocation + (Dir * Option->Distance);
	FVector BoxHalfSize = FVector(Option->BoxHalfSize, Option->BoxHalfSize, Option->BoxHalfSize);

	UKismetSystemLibrary::BoxTraceSingle(
		GetWorld(),
		StartLocation,
		EndLocation,
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

FHitResult UParkourComponent::LineTraceVer(FGameplayTag TagCategory, FVector Start, FVector Dir, bool bDrawDebug) const
{
	const UDA_TraceOptions* Option = GetTraceDA_Line(TagCategory);

	if (!Option) return FHitResult();

	FVector StartLocation = Start + FVector(0.f, 0.f, Option->ZOffset) + (Dir * Option->FrontOffset);
	FVector EndLocation = StartLocation + FVector(0.f, 0.f, Option->Distance);

	return LineTrace(StartLocation, EndLocation, bDrawDebug);
}

FHitResult UParkourComponent::LineTraceHor(FGameplayTag TagCategory, FVector Start, FVector Dir, bool bDrawDebug) const
{
	const UDA_TraceOptions* Option = GetTraceDA_Line(TagCategory);

	if (!Option) return FHitResult();

	FVector StartLocation = Start + FVector(0.f, 0.f, Option->ZOffset) + (Dir * Option->FrontOffset);
	FVector EndLocation = StartLocation + (Dir * Option->Distance);

	return LineTrace(StartLocation, EndLocation, bDrawDebug);
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

