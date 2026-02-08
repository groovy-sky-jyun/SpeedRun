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
	// 환경 태그 초기화
	CurrenEnvironmentTags.Reset();
	CurrenActionTags.Reset();

	// Landing 조건 확인 및 실행
	if (ParkourMovement->IsFalling())
	{
		/** 1.Detect Surface to Foot Under **/
		FVector FootLocation = Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		FVector FrontOffset = Player->GetActorForwardVector() * 10.f;
		FHitResult SurfaceHitResult = DetectToVerticalTraces(10, 30.f, -350.f, FootLocation + FrontOffset, Player->GetActorForwardVector(), 15.f, true);

		if (SurfaceHitResult.bBlockingHit)
		{
			/** 2.Landing Surface Space Check  **/
			FVector Start = SurfaceHitResult.ImpactPoint + (Player->GetActorForwardVector() * 150.f) + FVector(0.f, 0.f, 110.f);
			FVector End = Start + (Player->GetActorForwardVector() * 250.f);
			FHitResult HitResult = BoxTrace(Start, End, Player->GetActorRotation(), FVector(100.f, 100.f, 100.f));
			
			AddNewEnvironmentTag(TagCategory_SurfaceSpace, HitResult.bBlockingHit);
		}

		/** 3.Add Landing Action Tag **/
		FGameplayTag NewActionTag = FindActionTagByEnvironmentTags(TagCategory_Landing);
		if (NewActionTag.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("[ActionTag] : %s"), *NewActionTag.ToString());
			CurrenActionTags.AddTag(NewActionTag);
		}
		
		/** 4.Execute Landing Amin Montage **/
		/**************수정필요*****************/
		//TryParkourAction();
	}
	else
	{
		// 장애물 감지 
		FHitResult HitResult = IsDetectObstacle();

		if (HitResult.bBlockingHit)
		{
			AddNewEnvironmentTag(TagCategory_ObstacleHeight, GetObstacleHeightValue(HitResult)); //장애물 높이 태그
			AddNewEnvironmentTag(TagCategory_SurfaceGap, GetObstacleGapValue(HitResult)); //장애물 너비 태그
		}
		else
		{
			/** 1. Check Edge (Short Vertical Sphere Traces) **/
			FVector PlayerFootLocation = Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
			FHitResult EdgeHitResult = DetectToVerticalTraces(SurfaceGap_Count, SurfaceGap_Gap, -SurfaceGap_Height, PlayerFootLocation, Player->GetActorForwardVector(), 15.f, false);

			/** 2. Check Vertical Gap (Long Vertical Line Trace) **/
			if (EdgeHitResult.bBlockingHit)
			{
				
				FVector EdgeStart = EdgeHitResult.Location + (Player->GetActorForwardVector() * 10.f);
				FVector EdgeEnd = EdgeStart + FVector(0.f, 0.f, -100.f);

				FHitResult VerticalGapHitResult = LineTrace(EdgeStart, EdgeEnd);

				// [Step Height]
				if (VerticalGapHitResult.bBlockingHit) 
				{
					/** 3.Check Horizontal Gap (Long Horizontal Line Trace) **/
					FVector FrontOffset = Player->GetActorForwardVector() * 10.f;
					FVector StepOverStart = EdgeHitResult.ImpactPoint + FrontOffset + FVector(0.f, 0.f, -10.f);
					FVector StepOverEnd = StepOverStart + Player->GetActorForwardVector() * SurfaceGap_Distance;

					FHitResult HorizontalGapHitResult = LineTrace(StepOverStart, StepOverEnd);

					/** 4.Add Surface Gap Tag **/
					if (HorizontalGapHitResult.bBlockingHit)
					{
						AddNewEnvironmentTag(TagCategory_SurfaceGap, FVector::Dist(StepOverStart, HorizontalGapHitResult.ImpactPoint));
					}
				}
				else // [Rooftop]
				{
					/** 3.Get Horizontal Building Gap (Long Horizontal Sphere Traces) **/
					FVector BuildingEdgeStart = EdgeHitResult.Location + FVector(0.f, 0.f, 600.f) + (Player->GetActorForwardVector() * 10.f); //건물 건너편이 더 높은 경우를 위해 z +600.f 부터 시작
					FHitResult BuildingHorGapHitResult = DetectToHorizontalTraces(20, -100.f, 1200.f, BuildingEdgeStart, Player->GetActorForwardVector(), 20.f, true);

					/** 4.Add Surface Building Gap Tag **/
					if (BuildingHorGapHitResult.bBlockingHit)
					{
						/** 5.Get Vertical Building Height (Long Vertical Line Trace) **/
						FVector FrontOffset = -BuildingHorGapHitResult.ImpactNormal * 10.f;
						FVector BuildingHeightStart = BuildingHorGapHitResult.ImpactPoint + FVector(0.f, 0.f, 100.f) + FrontOffset;
						FVector BuildingHeightEnd = BuildingHeightStart + FVector(0.f, 0.f, -150.f);

						FHitResult BuildingVerGapHitResult = LineTrace(BuildingHeightStart, BuildingHeightEnd);

						/** 6.Add Surface Gap Tag (The distance from Rooftop A to Rooftop B) **/
						if (BuildingVerGapHitResult.bBlockingHit)
						{
							float BuildingGap = FVector::Dist(Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), BuildingVerGapHitResult.ImpactPoint);
							AddNewEnvironmentTag(TagCategory_BuildingGap, BuildingGap);
						}
					}
				}
			}
		}	

		// Execute Jump Action
		FGameplayTag NewActionTag = FindActionTagByEnvironmentTags(TagCategory_Jump);
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
	FGameplayTag JumpTag = FindCurrentActionTagForParentTag(TagCategory_Jump);

	// 2.해당 태그가 가지는 점프 설정 정보 가져오기
	FJumpOption Option = GetJumpOption(JumpTag);

	if (Option.JumpTagName.IsValid() && ParkourMovement)
	{
		// 3.점프 설정
		ParkourMovement->SetJumpValues(Option.GravityScale, Option.ZVelocity, Option.Impulse);
		Player->Jump();
	}
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
				// 3. 조건과 일치하는 ActionTag 찾기
				for (const auto& ConditionContainer : ActionEntry.ConditionTags)
				{
					if (CurrenEnvironmentTags.HasAll(ConditionContainer))
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
// 장애물 및 환경 감지 (Trace Logic)
//=================================
FHitResult UParkourComponent::IsDetectObstacle()
{
	/*
	* 점프 전 앞에 장애물이 있는지 체크 *
	*/

	// 1.Line Trace로 앞에 물체 감지
	float LocationZ = DetectZOffset - Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector Start = Player->GetActorLocation() + FVector(0.f, 0.f, LocationZ);
	FVector End = Start + (Player->GetActorForwardVector() * DetectDistance);

	// 2.감지 여부에 따른 태그 부여
	FGameplayTag NewDetectTag;
	FHitResult HitResult = LineTrace(Start, End);
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
	
	return HitResult;
}

float UParkourComponent::GetObstacleHeightValue(const FHitResult& DetectHitResult)
{
	FVector FrontOffset = Player->GetActorForwardVector() * 5.f;
	FVector Start = DetectHitResult.Location + FVector(0.f, 0.f, DetectHeight_ZOffset) + FrontOffset;
	FVector End = DetectHitResult.Location + FrontOffset;

	FHitResult HitResult = LineTrace(Start, End);
	if (HitResult.bBlockingHit)
	{
		return HitResult.Location.Z;
	}

	return -1.f;
}

float UParkourComponent::GetObstacleGapValue(const FHitResult& DetectHitResult)
{
	FVector TraceDirection = DetectHitResult.ImpactNormal * -1;
	FVector Start = DetectHitResult.ImpactPoint + FVector(0.f, 0.f, DetectHeight_ZOffset);
	FHitResult HitResult = DetectToVerticalTraces(DetectWidth_Count, DetectWidth_Gap, -DetectHeight_ZOffset, Start, TraceDirection, 15.f, false);

	if (HitResult.bBlockingHit)
	{
		return FVector::Dist(DetectHitResult.ImpactPoint + FVector(0.f, 0.f, DetectHeight_ZOffset), HitResult.TraceStart);
	}

	return -1.f;
}


float UParkourComponent::GetSurfaceGapValue()
{
	// 1.아래 떨어지는 지점 찾기
	FVector PlayerFootLocation = Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FHitResult LastHitResult = DetectToVerticalTraces(SurfaceGap_Count, SurfaceGap_Gap, -SurfaceGap_Height, PlayerFootLocation, Player->GetActorForwardVector(), 15.f, false);

	// 2.앞 방향 빈 공간 Gap Distance 구하기
	FVector FrontOffset = Player->GetActorForwardVector() * 10.f;
	FVector Start = LastHitResult.ImpactPoint + FrontOffset + FVector(0.f, 0.f, -10.f);
	FVector End = Start + Player->GetActorForwardVector() * SurfaceGap_Distance;

	FHitResult HitResult = LineTrace(Start, End);

	if (HitResult.bBlockingHit)
	{
		return FVector::Dist(Start, HitResult.ImpactPoint);
	}

	return -1.0f;
}

FHitResult UParkourComponent::DetectToHorizontalTraces(int32 TraceCount, float Gap, float Distance, FVector Start, FVector TraceDir, float Radius, bool bReturnHit) const
{
	FHitResult LastHitResult;

	for (int i = 0; i < TraceCount; i++)
	{
		FVector StartLocation = Start + FVector(0.f, 0.f, Gap * i);
		FVector EndLocation = StartLocation + TraceDir * Distance;

		FHitResult HitResult = SphereTrace(StartLocation, EndLocation, Radius);

		if (bReturnHit) //첫번째로 감지된 벽 감지 즉시 반환
		{
			//immediately return if find surface to floor.
			if (HitResult.bBlockingHit) return HitResult;
		}
		else { //공중 직전의 벽 높이 반환
			if (HitResult.bBlockingHit)
			{
				if (i == TraceCount - 1) //낭떨어지를 감지하지 못한 경우
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

FHitResult UParkourComponent::DetectToVerticalTraces(int32 TraceCount, float Gap, float Distance, FVector Start, FVector TraceDir, float Radius, bool bReturnHit) const
{
	FHitResult LastHitResult;

	for (int i = 0; i < TraceCount; i++)
	{
		FVector StartLocation = Start + TraceDir * Gap * i;
		FVector EndLocation = StartLocation + FVector(0.f, 0.f, Distance);

		FHitResult HitResult = SphereTrace(StartLocation, EndLocation, Radius);

		if (bReturnHit) //immediately return if find surface to floor.
		{
			if (HitResult.bBlockingHit) return HitResult;
		}
		else { //낭떨어지 직전의 바닥 위치 반환 (낭떨어지 감지)
			if (HitResult.bBlockingHit)
			{
				if (i == TraceCount - 1) //낭떨어지를 감지하지 못한 경우
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

FHitResult UParkourComponent::SphereTrace(const FVector& Start, const FVector& End, float Radius) const
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
		EDrawDebugTrace::ForDuration, // Draw Debug Type
		HitResult,
		true,          // Ignore Self
		FLinearColor::Red,   // 디버그 선 색상
		FLinearColor::Green, // 히트 시 색상
		5.0f           // 디버그 선 유지 시간
	);
	return HitResult;
}

FHitResult UParkourComponent::BoxTrace(const FVector& Start, const FVector& End, FRotator Rotation, FVector HalfSize) const
{
	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Player);

	UKismetSystemLibrary::BoxTraceSingle(
		GetWorld(), 
		Start,
		End,
		HalfSize, //FVector(가로, 세로, 높이)
		Rotation,
		UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1),
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResult,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		5.0f
	);
	
	return HitResult;
}

FHitResult UParkourComponent::LineTrace(const FVector& Start, const FVector& End) const
{
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Player);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_GameTraceChannel1, Params);
	DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 3.f);

	return HitResult;
}
