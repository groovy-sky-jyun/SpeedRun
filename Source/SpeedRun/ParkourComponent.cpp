// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourComponent.h"
#include "SpeedRunCharacter.h"
#include "ParkourMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "MotionWarpingComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameplayTagContainer.h"
#include "EnvironmentDataAsset.h"
#include "Kismet/KismetSystemLibrary.h"

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
	}
}


void UParkourComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UParkourComponent::TryParkourAction()
{
	/*
	* 파쿠르 액션 AnimMontage + Motion Warping 실행 *
	*/

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
	}
}


void UParkourComponent::HandleToJump()
{
	CurrenEnvironmentTags.Reset();

	FHitResult HitResult = IsDetectObstacle();
	if (HitResult.bBlockingHit)
	{
		AddNewEnvironmentTag(TagCategory_ObstacleHeight, GetObstacleHeightValue(HitResult));
		AddNewEnvironmentTag(TagCategory_SurfaceGap, GetObstacleGapValue(HitResult));
	}
	else
	{
		AddNewEnvironmentTag(TagCategory_SurfaceGap, GetSurfaceGapValue());
	}


}

FHitResult UParkourComponent::IsDetectObstacle()
{
	/*
	* 점프 전 앞에 장애물이 있는지 체크 *
	*/

	// 1.Line Trace로 앞에 물체 감지
	FHitResult HitResult;
	float LocationZ = DetectZOffset - Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector Start = Player->GetActorLocation() + FVector(0.f, 0.f, LocationZ);
	FVector End = Start + (Player->GetActorForwardVector() * DetectDistance);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_GameTraceChannel1);
	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 3.f, 0, 1.f);

	// 2.감지 여부에 따른 태그 부여
	FGameplayTag NewDetectTag;
	if (bHit)
	{
		NewDetectTag = Tag_DetectObstacle;
	}
	else
	{
		NewDetectTag = Tag_DetectNone;
	}

	CurrenEnvironmentTags.AddTag(NewDetectTag);
	
	return HitResult;
}

float UParkourComponent::GetObstacleGapValue(const FHitResult& DetectHitResult)
{
	FHitResult LastHitResult;
	FVector TraceDirection = DetectHitResult.ImpactNormal * -1;

	for (int i = 0; i < WidthTrace_Count; i++)
	{
		FVector FrontOffset = TraceDirection * (WidthTrace_Gap * i);
		FVector Start = DetectHitResult.ImpactPoint + FVector(0.f, 0.f, HeightTrace_ZOffset) + FrontOffset;
		FVector End = DetectHitResult.ImpactPoint + FrontOffset;

		FHitResult CurrentHitResult;
		if (SphereTrace(CurrentHitResult, Start, End, WidthTrace_SphereRadius))
		{
			LastHitResult = CurrentHitResult;
		}
		else
		{
			break;
		}
	}
	
	if (LastHitResult.bBlockingHit)
	{
		return FVector::Dist(DetectHitResult.ImpactPoint + FVector(0.f, 0.f, HeightTrace_ZOffset), LastHitResult.TraceStart);
	}

	return -1.f;
}

float UParkourComponent::GetObstacleHeightValue(const FHitResult& DetectHitResult)
{	
	FHitResult HitResult;
	FVector FrontOffset = Player->GetActorForwardVector() * 5.f;
	FVector Start = DetectHitResult.Location + FVector(0.f, 0.f, HeightTrace_ZOffset) + FrontOffset;
	FVector End = DetectHitResult.Location + FrontOffset;

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_GameTraceChannel1);
	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 5.f, 0, 1.f);

	if (bHit)
	{
		return HitResult.Location.Z;
	}

	return -1.f;
}

float UParkourComponent::GetSurfaceGapValue()
{
	// 1.아래로 떨어지는 지점 찾기
	FHitResult LastHitResult;
	for (int i = 0; i < HoleTrace_Count; i++)
	{
		FHitResult CurrentHitResult;
		FVector FrontOffset = Player->GetActorForwardVector() * HoleTrace_Gap * i;
		FVector Start = Player->GetActorLocation() + FVector(0.f, 0.f, -Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()) + FrontOffset;
		FVector End = Start + FVector(0.f, 0.f, -HoleTrace_Height);

		if (SphereTrace(CurrentHitResult, Start, End, HoleTrace_SphereRadius))
		{
			LastHitResult = CurrentHitResult;
		}
		else
		{
			break;
		}
	}

	// 2.앞 방향 빈 공간 Gap Distance 구하기
	FHitResult HitResult; 
	FVector FrontOffset = Player->GetActorForwardVector() * 5.f;
	FVector Start = LastHitResult.ImpactPoint + FrontOffset + FVector(0.f, 0.f, -5.f);
	FVector End = Start + Player->GetActorForwardVector() * HoleTrace_Distance;

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_GameTraceChannel1);
	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 5.f, 0, 1.f);

	if (bHit)
	{
		return FVector::Dist(Start, HitResult.ImpactPoint);
	}

	return -1.0f;
}

void UParkourComponent::AddNewEnvironmentTag(FGameplayTag TagCategory, float Value)
{
	CurrenEnvironmentTags.Reset();

	for (UEnvironmentDataAsset* DA : EnvironmentDataAssets)
	{
		// 1.유효성 확인
		if (!IsValid(DA)) continue;

		// 2.조건 일치하는 태그 추가
		if (DA->CategoryTag.MatchesTag(TagCategory))
		{
			for (const FEnvironmentState& TagDetails : DA->TagList)
			{
				if (TagDetails.MinValue < Value && Value <= TagDetails.MaxValue)
				{
					CurrenEnvironmentTags.AddTag(TagDetails.Tag);
					UE_LOG(LogTemp, Warning, TEXT("Add New Environment Tag : %s"), *TagDetails.Tag.ToString());
					return;
				}
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("New Environment Tag is None"));
}

bool UParkourComponent::SphereTrace(FHitResult& HitResult, FVector Start, FVector End, float Radius)
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Player);

	bool bHit = UKismetSystemLibrary::SphereTraceSingle(
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
		FLinearColor::Green,   // 디버그 선 색상
		FLinearColor::Red, // 히트 시 색상
		5.0f           // 디버그 선 유지 시간
	);

	return bHit;
}

