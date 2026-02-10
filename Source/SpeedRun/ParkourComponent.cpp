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
	UE_LOG(LogTemp, Warning, TEXT("======RESET======"));
	CurrenEnvironmentTags.Reset();
	// Landing 조건 확인 및 실행
	if (ParkourMovement->IsFalling())
	{
		DoLanding();
	}
	else
	{
		// 태그 초기화
		UE_LOG(LogTemp, Warning, TEXT("======RESET======"));
		CurrenActionTags.Reset();

		// 장애물 감지 
		bool bObstacleHit = DetectObstacle();

		if (bObstacleHit)
		{
			/** 1.Add Obstacle Height (Horizontal Sphere Traces) */
			FVector ObstacleHeightStart = Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
			FHitResult HeightHitResult = DetectToHorTraces(Tag_ObstacleHeight, ObstacleHeightStart, Player->GetActorForwardVector(), false, false);

			if (HeightHitResult.bBlockingHit)
			{
				AddNewEnvironmentTag(Tag_ObstacleHeight, HeightHitResult.ImpactPoint.Z);
			}

			/** 2.Add Obstacle Width (Vertical Sphere Traces) */
			FVector ObstacleWidthStart = HeightHitResult.ImpactPoint;
			FHitResult WidthHitResult = DetectToVerTraces(Tag_ObstacleWidth, ObstacleWidthStart, Player->GetActorForwardVector(), false, false);
			float ObstacleWidth = FVector::Dist(ObstacleWidthStart, WidthHitResult.ImpactPoint);

			if (WidthHitResult.bBlockingHit)
			{
				AddNewEnvironmentTag(Tag_ObstacleWidth, ObstacleWidth);
			}
		}
		else
		{
			/** 1. Check Edge (Short Vertical Sphere Traces) **/
			FVector PlayerFootLocation = Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
			FHitResult EdgeHitResult = DetectToVerTraces(Tag_DetectNone, PlayerFootLocation, Player->GetActorForwardVector(), false, false);

			/** 2. Check 'Step Box' or 'Rooftop'? (Long Vertical Line Trace) **/
			if (EdgeHitResult.bBlockingHit)
			{
				FVector EdgeStart = EdgeHitResult.ImpactPoint;
				FHitResult EdgeHeightHitResult = LineTraceVer(Tag_DetectNone, EdgeStart, Player->GetActorForwardVector(), false);

				// [Step Box]
				if (EdgeHeightHitResult.bBlockingHit)
				{
					/** 3.Add StepBox Width (Long Horizontal Line Trace) **/
					FVector StepOverStart = EdgeHeightHitResult.ImpactPoint;
					FHitResult StepWidthHitResult = LineTraceHor(Tag_StepBoxGap, StepOverStart, Player->GetActorForwardVector(), false);

					if (StepWidthHitResult.bBlockingHit)
					{
						AddNewEnvironmentTag(Tag_StepBoxGap, FVector::Dist(StepOverStart, StepWidthHitResult.ImpactPoint));
					}
				}
				else // [Rooftop]
				{
					/** 3.Get Horizontal Building Gap (Long Horizontal Sphere Traces) **/
					FVector BuildingEdgeStart = Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
					FHitResult BuildingHorHitResult = DetectToHorTraces(Tag_BuildingGap, PlayerFootLocation, Player->GetActorForwardVector(), true, false);

					if (BuildingHorHitResult.bBlockingHit)
					{
						/** 4.Get Vertical Building Height (Long Vertical Line Trace) **/
						FVector BuildingHeightStart = BuildingHorHitResult.ImpactPoint;
						FHitResult BuildingVerHitResult = LineTraceVer(Tag_BuildingGap, BuildingHeightStart, -BuildingHorHitResult.ImpactNormal, false);

						/** 5.Add Surface Gap (The distance from Rooftop A to Rooftop B) **/
						if (BuildingVerHitResult.bBlockingHit)
						{
							float BuildingGap = FVector::Dist(Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), BuildingVerHitResult.ImpactPoint);
							AddNewEnvironmentTag(Tag_BuildingGap, BuildingGap);
						}
					}
				}
			}
		}	

		// Execute Jump Action
		FGameplayTag NewActionTag = FindActionTagByEnvironmentTags(Tag_Jump);
		if (NewActionTag.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("[ActionTag] : %s"), *NewActionTag.ToString());

			/** 1.Add ActionTag **/
			CurrenActionTags.AddTag(NewActionTag);

			/** 2.Apply Jump Config and Play Jump **/
			DoParkourJump();
		}
		else
		{
			Player->Jump();
		}
	}
}


void UParkourComponent::DoParkourJump()
{
	// 1.현재 액션 컨테이너에서 점프 관련 태그 찾기
	FGameplayTag JumpTag = FindCurrentActionTagForParentTag(Tag_Jump);

	// 2.해당 태그가 가지는 점프 설정 정보 가져오기
	FJumpOption Option = GetJumpOption(JumpTag);

	if (Option.JumpTagName.IsValid() && ParkourMovement)
	{
		// 3.점프 설정
		ParkourMovement->SetJumpValues(Option.GravityScale, Option.ZVelocity, Option.Impulse);
		Player->Jump();
	}
}

void UParkourComponent::DoLanding()
{
	/** 1.Detect Surface to Foot Under **/
	FVector FootLocation = Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FHitResult SurfaceHitResult = DetectToVerTraces(Tag_SurfaceSpace, FootLocation, Player->GetActorForwardVector(), true, true);

	if (SurfaceHitResult.bBlockingHit)
	{
		/** 2.Landing Surface Space Check  **/
		FHitResult HitResult = BoxTrace(Tag_SurfaceSpace, SurfaceHitResult.ImpactPoint, Player->GetActorForwardVector(), Player->GetActorRotation(), true);

		AddNewEnvironmentTag(Tag_SurfaceSpace, HitResult.bBlockingHit);
	}

	/** 3.Add Landing Action Tag **/
	FGameplayTag NewActionTag = FindActionTagByEnvironmentTags(Tag_Landing);
	if (NewActionTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ActionTag] : %s"), *NewActionTag.ToString());

		/** 1.Add ActionTag **/
		CurrenActionTags.AddTag(NewActionTag);
	}
	
	/** 4.Execute Landing Amin Montage **/
	/**************수정필요*****************/
	//TryParkourAction();
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
	CurrenEnvironmentTags.AddTag(NewDetectTag);

	return HitResult.bBlockingHit;
}


//=================================
//           Tag 결정        
//=================================
FGameplayTag UParkourComponent::FindCurrentActionTagForParentTag(FGameplayTag ParentTag) const
{
	if (!CurrenActionTags.IsEmpty())
	{
		for (const auto& Tag : CurrenActionTags)
		{
			if (Tag.MatchesTag(ParentTag))
			{
				return Tag;
			}
		}
	}

	return FGameplayTag::EmptyTag;
}

void UParkourComponent::AddNewEnvironmentTag(FGameplayTag TagCategory, float Value)
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
				CurrenEnvironmentTags.AddTag(TagDetails.Tag);
				UE_LOG(LogTemp, Warning, TEXT("[EnvironmentTag] : %s"), *TagDetails.Tag.ToString());
				return;
			}
		}
	}
}

FGameplayTag UParkourComponent::FindActionTagByEnvironmentTags(FGameplayTag ActionCategory) const
{
	FGameplayTagContainer HaveTags = CurrenActionTags;
	HaveTags.AppendTags(CurrenEnvironmentTags);

	if (!ActionCategoryList.IsEmpty())
	{
		TArray<FParkourActionList> ActionList;
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
	}

	return FGameplayTag::EmptyTag;
}


FJumpOption UParkourComponent::GetJumpOption(FGameplayTag NewTag) const
{
	if (JumpConfigs)
	{
		for (const FJumpOption& List : JumpConfigs->JumpList)
		{
			if (NewTag.MatchesTag(List.JumpTagName))
			{
				return List;
			}
		}
	}

	FJumpOption DefaultOption;
	return DefaultOption;
}


//=================================
//      Basic Trace Logic
//=================================
const UDA_SphereTracesOption* UParkourComponent::FindSpheresTraceOption(FGameplayTag TagCategory) const
{
	for (auto& Option : SphereTracesOptions)
	{
		if (Option->CategoryTag.MatchesTag(TagCategory))
		{
			return Option;
		}
	}
	return nullptr;
}

const UDA_BoxTraceOption* UParkourComponent::FindBoxTraceOption(FGameplayTag TagCategory) const
{
	for (auto& Option : BoxTraceOptions)
	{
		if (Option->CategoryTag.MatchesTag(TagCategory))
		{
			return Option;
		}
	}
	return nullptr;
}

const UDA_TraceOptions* UParkourComponent::FindLineTraceOption(FGameplayTag TagCategory) const
{
	for (auto& Option : LineTraceOptions)
	{
		if (Option->CategoryTag.MatchesTag(TagCategory))
		{
			return Option;
		}
	}
	return nullptr;
}

FHitResult UParkourComponent::DetectToHorTraces(FGameplayTag TagCategory, FVector Start, FVector Dir, bool bReturnHit, bool bDrawDebug) const
{
	const UDA_SphereTracesOption* Option = FindSpheresTraceOption(TagCategory);

	FHitResult LastHitResult;

	for (int i = 0; i < Option->Count; i++)
	{
		FVector StartLocation = Start + (Dir * Option->FrontOffset) + FVector(0.f, 0.f, Option->ZOffset + (Option->Gap * i));
		FVector EndLocation = StartLocation + Dir * Option->Distance;

		FHitResult HitResult = SphereTrace(StartLocation, EndLocation, Option->Radius, bDrawDebug);

		if (bReturnHit) //첫번째로 감지된 벽 감지 즉시 반환
		{
			//immediately return if find surface to floor.
			if (HitResult.bBlockingHit) return HitResult;
		}
		else { //공중 직전의 벽 높이 반환
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

FHitResult UParkourComponent::DetectToVerTraces(FGameplayTag TagCategory, FVector Start, FVector Dir, bool bReturnHit, bool bDrawDebug) const
{
	const UDA_SphereTracesOption* Option = FindSpheresTraceOption(TagCategory);

	FHitResult LastHitResult;

	for (int i = 0; i < Option->Count; i++)
	{
		FVector StartLocation = Start + FVector(0.f, 0.f, Option->ZOffset) + (Dir * (Option->FrontOffset + (Option->Gap * i)));
		FVector EndLocation = StartLocation + FVector(0.f, 0.f, Option->Distance);

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
	const UDA_BoxTraceOption* Option = FindBoxTraceOption(TagCategory);

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
	const UDA_TraceOptions* Option = FindLineTraceOption(TagCategory);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Player);

	FVector StartLocation = Start + FVector(0.f, 0.f, Option->ZOffset) + (Dir * Option->FrontOffset);
	FVector EndLocation = StartLocation + FVector(0.f, 0.f, Option->Distance);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_GameTraceChannel1, Params);

	if (bDrawDebug)
	{
		DrawDebugLine(GetWorld(), StartLocation, EndLocation, bHit ? FColor::Green : FColor::Red, false, 3.f);
	}

	return HitResult;
}

FHitResult UParkourComponent::LineTraceHor(FGameplayTag TagCategory, FVector Start, FVector Dir, bool bDrawDebug) const
{
	const UDA_TraceOptions* Option = FindLineTraceOption(TagCategory);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Player);

	FVector StartLocation = Start + FVector(0.f, 0.f, Option->ZOffset) + (Dir * Option->FrontOffset);
	FVector EndLocation = StartLocation + (Dir * Option->Distance);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_GameTraceChannel1, Params);

	if (bDrawDebug)
	{
		DrawDebugLine(GetWorld(), StartLocation, EndLocation, bHit ? FColor::Green : FColor::Red, false, 3.f);
	}

	return HitResult;
}
