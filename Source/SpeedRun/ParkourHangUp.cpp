// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourHangUp.h"
#include "SpeedRunCharacter.h"
#include "ParkourMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "MotionWarpingComponent.h"
#include "Animation/AnimMontage.h"
#include "ParkourManager.h"
#include "TimerManager.h"


void UParkourHangUp::Initialize(ASpeedRunCharacter* OwnerPlayer, UParkourManager* ParkourComponent)
{
	Super::Initialize(OwnerPlayer, ParkourComponent);
}

bool UParkourHangUp::CheckVisibleToAction()
{
	if (ParkourManager->GetIsOnLedge())
	{		
		/* 1. Ledge 위에 올라설 공간 있는지 확인 */
		FHitResult HitResult;

		FVector ForwardVector = Player->GetActorLocation() + (Player->GetActorForwardVector() * 85.f);
		float ZOffset = CapsuleHalfHeight + 100.f;

		FVector TargetVector = ForwardVector + FVector(0.f, 0.f, ZOffset);

		bool bIsFullTopSurface = GetWorld()->SweepSingleByChannel(HitResult, TargetVector, TargetVector, FQuat::Identity, ECollisionChannel::ECC_GameTraceChannel1, FCollisionShape::MakeSphere(25.f));
		//DrawDebugSphere(GetWorld(), TargetVector, 25.f, 12, FColor::Green, false, 10.0f);

		if (!bIsFullTopSurface) 
		{
			return true;
		}
	}

	return false;
}

void UParkourHangUp::OnStart()
{
	/* 2. 올라갈 Ledge 높이 확인 (Top Surface 높이 확인) */
	FHitResult HitResultSurface;

	FVector ForwardVector = Player->GetActorLocation() + (Player->GetActorForwardVector() * 85.f);
	float ZOffset = CapsuleHalfHeight + 100.f;

	FVector TargetVector = ForwardVector + FVector(0.f, 0.f, ZOffset);

	bool bHitSurface = GetWorld()->LineTraceSingleByChannel(HitResultSurface, TargetVector, TargetVector + FVector(0.f, 0.f, -100.f), ECollisionChannel::ECC_GameTraceChannel1);
	//DrawDebugLine(GetWorld(), TargetVector, TargetVector + FVector(0.f, 0.f, -100.f), FColor::Purple, false, 2.f);


	/* 3. 캐릭터 물리 상태 & 위치 조정 */
	Player->SetActorEnableCollision(false);

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UParkourHangUp::SetFly, 0.8f, false);

	FVector WorldLocation = CapsuleComponent->GetComponentLocation();
	FVector LocationOffset = WorldLocation + FVector(0.f, 0.f, 70.f) + (Player->GetActorForwardVector() * 15.f);
	FRotator TargetRotator = CapsuleComponent->GetComponentRotation();

	Player->SetActorLocationAndRotation(LocationOffset, TargetRotator);

	ParkourManager->SetOverrideHandIK(false);
	ParkourManager->SetOverrideFootIK(false);

	FVector TargetLocation = HitResultSurface.ImpactPoint + (Player->GetActorForwardVector() * 25.f) + FVector(0.f, 0.f, 25.f);
	//DrawDebugSphere(GetWorld(), TargetLocation, 10.f, 12, FColor::Green, false, 10.0f);

	WarpComponent->AddOrUpdateWarpTargetFromLocationAndRotation(FName("MantlePosition"), TargetLocation, Player->GetActorRotation());

	AnimInstance->Montage_Play(MantleMontage);

	
	
	//
}

void UParkourHangUp::OnEnd()
{
	ParkourManager->SetIsOnLedge(false);
}

void UParkourHangUp::SetFly()
{
	Movement->SetMovementMode(EMovementMode::MOVE_Falling);
}
