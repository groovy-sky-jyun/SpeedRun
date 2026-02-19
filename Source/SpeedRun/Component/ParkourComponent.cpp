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

	if (Player)
	{
		AnimInstance = Player->GetMesh()->GetAnimInstance();
		WarpComponent = Player->GetMotionWarpingComponent();
		ParkourMovement = Player->GetParkourMovement();
	}

	bCanLanding = false;
	InitTraceMap();
	InitTagMap();
}


void UParkourComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!ParkourMovement->IsFalling() && bCanLanding)
	{
		PlayAminMontage(Tag_Landing);
		bCanLanding = false;
	}
}

//=================================
//      액션 수행 (Execution)         
//=================================
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

		/** 1.태그 초기화 **/
		CurrentActionTags.Reset();

		/** 2.환경 감지 및 태그 획득 **/
		ScanEnvironment();

		/** 3.액션 태그 획득**/
		FGameplayTag JumpTag = SelectActionTagOnContext(Tag_Jump);

		/** 4.액션 옵션 적용 및 실행 **/
		if (JumpTag.MatchesTag(Tag_Vault)) //Vault 설정
		{
			bCanVault = true;
		}
		
		FJumpOption Option = JumpConfigMap.FindRef(JumpTag);
		if (Option.JumpTagName.IsValid() && ParkourMovement) //Jump Option 설정
		{
			ParkourMovement->SetJumpValues(Option.GravityScale, Option.ZVelocity, Option.Impulse);
		}

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
		/** 2.Detect Obstacle from Landing Surface Space **/
		FHitResult HitResult = BoxTrace(Tag_SurfaceSpace, SurfaceHitResult.ImpactPoint, Player->GetActorForwardVector(), Player->GetActorRotation(), true);
		
		/** 3.Add Landing Surface Space Tag **/
		SelectEnvTagOnContext(Tag_SurfaceSpace, HitResult.bBlockingHit);

		/** 4.Add Landing Action Tag **/
		SelectActionTagOnContext(Tag_Landing);

		/** 5.Execute Landing Amin Montage **/
		bCanLanding = true;
	}
	return;
}

void UParkourComponent::PlayAminMontage(FGameplayTag TagCategory)
{
	FAnimInfo ActionData = FindAnimInfo(TagCategory);
	if (!ActionData.TagName.IsValid()) return;

	if (UAnimMontage* AnimMontage = ActionData.AnimMontage)
	{
		AnimInstance->Montage_Play(AnimMontage, ActionData.SpeedRate);
	}
}

//=================================
//   Find DataAsset (feat.Tag)
//=================================
FGameplayTag UParkourComponent::SelectEnvTagOnContext(const FGameplayTag& TagCategory, float Value) 
{
	UDA_EnvironmentTags* DA = EnvironmentMap.FindRef(TagCategory);
	if (!DA) return FGameplayTag::EmptyTag;

	for (const FEnvironmentState& TagDetails : DA->TagList)
	{
		if (Value > TagDetails.MinValue && Value <= TagDetails.MaxValue)
		{
			UE_LOG(LogTemp, Warning, TEXT("[EnvironmentTag] : %s"), *TagDetails.Tag.ToString());
			CurrentEnvironmentTags.AddTag(TagDetails.Tag);
			return TagDetails.Tag;
		}
	}

	return FGameplayTag::EmptyTag;
}

FGameplayTag UParkourComponent::SelectActionTagOnContext(const FGameplayTag& ActionCategory) 
{
	UDA_ParkourActionCategory* DA = ActionCategoryMap.FindRef(ActionCategory);
	if (!DA) return FGameplayTag::EmptyTag;

	FGameplayTagContainer HaveTags = CurrentActionTags;
	HaveTags.AppendTags(CurrentEnvironmentTags);

	// 액션 카테고리와 일치하는 DA 찾기
	for (const auto& ActionEntry : DA->ActionList)
	{
		// 3. Action Condition과 CurrentTags가 일치하는 ActionTag 찾기
		for (const auto& ConditionContainer : ActionEntry.ConditionTags)
		{
			if (HaveTags.HasAll(ConditionContainer))
			{
				UE_LOG(LogTemp, Warning, TEXT("[ActionTag] : %s"), *ActionEntry.ActionTag.ToString());
				CurrentActionTags.AddTag(ActionEntry.ActionTag);
				return ActionEntry.ActionTag;
			}
		}
	}

	return FGameplayTag::EmptyTag;
}

FAnimInfo UParkourComponent::FindAnimInfo(const FGameplayTag& TagCategory) const
{
	UDA_AnimOption* DA = AnimInfoMap.FindRef(TagCategory);
	if (!DA) return FAnimInfo();

	FGameplayTagContainer Filtered = CurrentActionTags.Filter(FGameplayTagContainer(TagCategory));

	for (const auto& Conditions : DA->TagList)
	{
		if (Conditions.TagName.MatchesTag(Filtered.First()))
		{
			return Conditions;
		}
	}
	return FAnimInfo();
}


//=================================
//      장애물 및 환경 감지 
//=================================
void UParkourComponent::ScanEnvironment()
{
	/** 1.정면 장애물 감지 **/
	bool bObstacleHit = DetectObstacle();

	if (bObstacleHit)
	{
		/** 2.장애물 스캔 **/
		ScanObstacleContext();
	}
	else
	{
		/** 2.Edge 스캔 **/
		ScanEdgeContext();
	}
}

bool UParkourComponent::DetectObstacle()
{
	// Detect FrontObstacle (LineTrace_Horizontal)
	FVector Start = Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FHitResult HitResult = LineTrace(Tag_Detect, ETraceDirection::Horizontal, Start, Player->GetActorForwardVector(), false);

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

void UParkourComponent::ScanObstacleContext()
{
	/** 1.Add Obstacle Height (Horizontal Sphere Traces) */
	FVector ObstacleHeightStart = Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FHitResult HeightHitResult = DetectSphereTraces(Tag_ObstacleHeight, ObstacleHeightStart, Player->GetActorForwardVector(), false, ETraceDirection::Horizontal, true);

	if (HeightHitResult.bBlockingHit)
	{
		float Height = HeightHitResult.ImpactPoint.Z - ObstacleHeightStart.Z;
		SelectEnvTagOnContext(Tag_ObstacleHeight, Height);
	}

	/** 2.Add Obstacle Width (Vertical Sphere Traces) */
	FVector ObstacleWidthStart = HeightHitResult.ImpactPoint;
	FHitResult WidthHitResult = DetectSphereTraces(Tag_ObstacleWidth, ObstacleWidthStart, Player->GetActorForwardVector(), false, ETraceDirection::Vertical, false);
	float ObstacleWidth = FVector::Dist(ObstacleWidthStart, WidthHitResult.ImpactPoint);


	if (WidthHitResult.bBlockingHit)
	{
		SelectEnvTagOnContext(Tag_ObstacleWidth, ObstacleWidth);

		/** 3.Add Obstacle Land = SurfaceHeight (Vertical Line Trace) **/
		FVector EdgeStart = WidthHitResult.ImpactPoint;
		FHitResult EdgeHeightHitResult = LineTrace(Tag_ObstacleLand, ETraceDirection::Vertical, EdgeStart, Player->GetActorForwardVector(), true);
		if (EdgeHeightHitResult.bBlockingHit)
		{
			SelectEnvTagOnContext(Tag_ObstacleLand, FVector::Dist(EdgeHeightHitResult.TraceStart, EdgeHeightHitResult.Location));
		}
		else //Obstacle.Land.Abyss
		{
			SelectEnvTagOnContext(Tag_ObstacleLand, 10000);
		}
	}
}

void UParkourComponent::ScanEdgeContext()
{
	/** 1. Check Edge (Short Vertical Sphere Traces) **/
	FVector PlayerFootLocation = Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FHitResult EdgeHitResult = DetectSphereTraces(Tag_DetectNone, PlayerFootLocation, Player->GetActorForwardVector(), false, ETraceDirection::Vertical, false);

	/** 2. Check 'Step Box' or 'Rooftop'? (Long Vertical Line Trace) **/
	if (EdgeHitResult.bBlockingHit)
	{
		FVector EdgeStart = EdgeHitResult.ImpactPoint;
		FHitResult EdgeHeightHitResult = LineTrace(Tag_DetectNone, ETraceDirection::Vertical, EdgeStart, Player->GetActorForwardVector(), false);

		// [Step Box]
		if (EdgeHeightHitResult.bBlockingHit)
		{
			ScanStepBoxContext(EdgeHeightHitResult.ImpactPoint);

		}
		else // [Rooftop]
		{
			ScanBuildingContext(PlayerFootLocation);
		}
	}
}

void UParkourComponent::ScanStepBoxContext(const FVector& StepOverStart)
{
	/** 3.Add StepBox Width (Long Horizontal Line Trace) **/
	FHitResult StepWidthHitResult = LineTrace(Tag_StepBoxGap, ETraceDirection::Horizontal, StepOverStart, Player->GetActorForwardVector(), false);

	if (StepWidthHitResult.bBlockingHit)
	{
		SelectEnvTagOnContext(Tag_StepBoxGap, FVector::Dist(StepOverStart, StepWidthHitResult.ImpactPoint));
	}
}

void UParkourComponent::ScanBuildingContext(const FVector& PlayerFootLocation)
{
	/** 3.Get Horizontal Building Gap (Long Horizontal Sphere Traces) **/
	FVector BuildingEdgeStart = Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FHitResult BuildingHorHitResult = DetectSphereTraces(Tag_BuildingGap, PlayerFootLocation, Player->GetActorForwardVector(), true, ETraceDirection::Horizontal, false);

	if (BuildingHorHitResult.bBlockingHit)
	{
		/** 4.Get Vertical Building Height (Long Vertical Line Trace) **/
		FVector BuildingHeightStart = BuildingHorHitResult.ImpactPoint;
		FHitResult BuildingVerHitResult = LineTrace(Tag_BuildingGap, ETraceDirection::Vertical, BuildingHeightStart, -BuildingHorHitResult.ImpactNormal, false);

		/** 5.Add Surface Gap (The distance from Rooftop A to Rooftop B) **/
		if (BuildingVerHitResult.bBlockingHit)
		{
			float BuildingGap = FVector::Dist(Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), BuildingVerHitResult.ImpactPoint);
			SelectEnvTagOnContext(Tag_BuildingGap, BuildingGap);
		}
	}
}


//=================================
//      Basic Trace Logic
//=================================
FHitResult UParkourComponent::DetectSphereTraces(FGameplayTag TagCategory, FVector Start, FVector Dir, bool bReturnHit, ETraceDirection TraceDir, bool bDrawDebug) const
{
	const UDA_SphereTracesOption* Option = SphereTraceMap.FindRef(TagCategory);

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
	const UDA_BoxTraceOption* Option = BoxTraceMap.FindRef(TagCategory);

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

FHitResult UParkourComponent::LineTrace(FGameplayTag TagCategory, ETraceDirection TraceDir, FVector Start, FVector Dir, bool bDrawDebug) const
{
	const UDA_LineTraceOption* Option = LineTraceMap.FindRef(TagCategory);
	if (!Option) return FHitResult();

	FVector StartLocation, EndLocation;
	if (TraceDir == ETraceDirection::Vertical)
	{
		StartLocation = Start + FVector(0.f, 0.f, Option->ZOffset) + (Dir * Option->FrontOffset);
		EndLocation = StartLocation + FVector(0.f, 0.f, Option->Distance);
	}
	else if (TraceDir == ETraceDirection::Horizontal)
	{
		StartLocation = Start + FVector(0.f, 0.f, Option->ZOffset) + (Dir * Option->FrontOffset);
		EndLocation = StartLocation + (Dir * Option->Distance);
	}

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Player);
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_GameTraceChannel1, Params);

	if (bDrawDebug)
	{
		DrawDebugLine(GetWorld(), StartLocation, EndLocation, bHit ? FColor::Green : FColor::Red, false, 3.f);
	}

	return HitResult;
}

void UParkourComponent::InitTraceMap()
{
	SphereTraceMap.Empty();
	BoxTraceMap.Empty();
	LineTraceMap.Empty();

	for (const auto& Option : TracesOptions)
	{
		if (!Option) continue;

		if (auto* SphereTrace = Cast<UDA_SphereTracesOption>(Option))
		{
			SphereTraceMap.Add(SphereTrace->CategoryTag, SphereTrace);
		}
		else if (auto* BoxTrace = Cast<UDA_BoxTraceOption>(Option))
		{
			BoxTraceMap.Add(BoxTrace->CategoryTag, BoxTrace);
		}
		else if(auto* LineTrace = Cast<UDA_LineTraceOption>(Option))
		{
			LineTraceMap.Add(LineTrace->CategoryTag, LineTrace);
		}
	}
}

void UParkourComponent::InitTagMap()
{
	CHECK_ARRAY(DA_EnvironmentTags);   
	CHECK_ARRAY(DA_ActionCategoryList);
	CHECK_ARRAY(DA_ActionAnimInfo);
	CHECK_VALID(DA_JumpConfig);

	EnvironmentMap.Empty();
	for (const auto& Option : DA_EnvironmentTags)
	{
		if (!Option) continue;
		EnvironmentMap.Add(Option->CategoryTag, Option);
	}

	ActionCategoryMap.Empty();
	for (const auto& Option : DA_ActionCategoryList)
	{
		if (!Option) continue;
		ActionCategoryMap.Add(Option->ActionCategory, Option);
	}

	AnimInfoMap.Empty();
	for (const auto& Option : DA_ActionAnimInfo)
	{
		if (!Option) continue;
		AnimInfoMap.Add(Option->CategoryTag, Option);
	}

	JumpConfigMap.Empty();
	for (const auto& Option : DA_JumpConfig->JumpList)
	{
		if (!Option.JumpTagName.IsValid()) continue;
		JumpConfigMap.Add(Option.JumpTagName, Option);
	}
}

