// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourHang.h"
#include "SpeedRunCharacter.h"
#include "ParkourMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "ParkourManager.h"
#include "Animation/AnimMontage.h"
#include "MotionWarpingComponent.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "MotionWarpingComponent.h"

void UParkourHang::Initialize(ASpeedRunCharacter* OwnerPlayer, UParkourManager* ParkourComponent)
{
	Super::Initialize(OwnerPlayer, ParkourComponent);
}

bool UParkourHang::CheckVisibleToAction()
{
	DetectWall();
	return false;
	/*
	float InitialZOffset = Movement->IsFalling() ? InitialZOffset_Falling : InitialZOffset_Grounded;
	float TraceVertical = Movement->IsFalling() ? TraceVertical_Falling : TraceVertical_Grounded;

	if (Movement->IsFalling())
	{
		UE_LOG(LogTemp, Warning, TEXT("IS FALLING"));
	}
	return CheckDetectToLedge(InitialZOffset, TraceDistanceH, TraceVertical);*/
}

void UParkourHang::OnStart()
{
	HangOnLedge();
}

void UParkourHang::OnUpdate()
{
	UE_LOG(LogTemp, Warning, TEXT("ParkourHang::OnUpdate"));
}

void UParkourHang::OnEnd()
{
	UE_LOG(LogTemp, Warning, TEXT("ParkourHang::OnEnd"));
}

bool UParkourHang::CheckDetectToLedge(float InitialZOffset, float TraceDistance, float TraceVertical, float RightOffset)
{
	FHitResult HitResult_H;
	int TraceMax_H = 8;
	int TraceIndex;
	float SphereRadius = 6.f;

	// Check for Ledge.(Horizontal)
	for (TraceIndex = 0; TraceIndex < TraceMax_H; TraceIndex++)
	{
		FHitResult HitResult;

		float ZOffset = InitialZOffset + (TraceIndex * TraceVertical);
		FVector RightVector = Player->GetActorRightVector() * RightOffset * 40.f;

		FVector StartLocation = Player->GetActorLocation() + FVector(0.f, 0.f, ZOffset) + RightVector;
		FVector EndLocation = StartLocation + (Player->GetActorForwardVector() * TraceDistance);

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Player);

		bool bHorizontalHit = GetPlayerWorld()->SweepSingleByChannel(HitResult, StartLocation, EndLocation, FQuat::Identity, ECollisionChannel::ECC_GameTraceChannel1, FCollisionShape::MakeSphere(SphereRadius), Params);

		FRotator CapsuleRotator = Player->GetActorForwardVector().Rotation();
		CapsuleRotator.Pitch += 90.f;
		
		if (!bHorizontalHit) break;
		else 
		{
			DrawDebugCapsule(GetPlayerWorld(), (StartLocation + HitResult.Location) * 0.5, TraceDistance * 0.5, SphereRadius, CapsuleRotator.Quaternion(), FColor::Blue, false, 2.0f);

			HitResult_H = HitResult; 
		}
	}

	// Check for Surface. (Vertical)
	if (0 < TraceIndex && TraceIndex < TraceMax_H)
	{
		FHitResult HitResult_V;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Player);
		bool bVerticalHit = GetPlayerWorld()->SweepSingleByChannel(HitResult_V, HitResult_H.ImpactPoint + FVector(0.f, 0.f, 20.f), HitResult_H.ImpactPoint, FQuat::Identity, ECollisionChannel::ECC_GameTraceChannel1, FCollisionShape::MakeSphere(SphereRadius), Params);

		DrawDebugSphere(GetPlayerWorld(), HitResult_V.Location, SphereRadius, 12, FColor::Yellow, false, 2.0f);

		// if Distance is 0, the trace started inside an wall (InitialOverlap).
		if (bVerticalHit && HitResult_V.Distance > 0)
		{
			DetectLedgeLocation = HitResult_V.ImpactPoint;
			DetectLedgeNormal = HitResult_H.ImpactNormal;
			return true;
		}
	}

	DetectLedgeLocation = FVector(0.f, 0.f, 0.f);
	DetectLedgeNormal = FVector(0.f, 0.f, 0.f);
	return false;
}

void UParkourHang::HangOnLedge()
{
	FRotator TargetRotator;

	FVector TargetLocation;


	// 1. 사전 설정
	ParkourManager->SetCanMove(false);
	CheckLedfeSurfaceHandle.Invalidate();


	// 2. Target Rotation, Location 설정
	CalculateLedgeRotatorAndLocation(TargetRotator, TargetLocation);


	// 4-1. 발 지지대 공간 확인
	CheckIfBelowLedgeHasSurface();
	FindLedgeHandIKLocation();

	// 3. Rotation 적용
	Player->SetActorRotation(TargetRotator);

	
	// 4. MotionWarping 설정
	ParkourManager->SetOverrideFootIK(true);

	if (!Movement->IsFalling())
	{
		Movement->SetMovementMode(EMovementMode::MOVE_Flying);

		if (!ParkourManager->GetIsOnLedge())
		{
			FRotator OffsetRotator = UKismetMathLibrary::MakeRotFromX(DetectLedgeNormal);

			float HandOffset = CapsuleHalfHeight + 110.f;
			FVector HeightLocation = DetectLedgeLocation - FVector(0.f, 0.f, HandOffset);

			FVector ForwardDir = UKismetMathLibrary::GetForwardVector(OffsetRotator);
			FVector AwayOffset = ForwardDir * 90.0f;

			FVector InitLocation = HeightLocation + AwayOffset;

			Player->SetActorLocationAndRotation(InitLocation, TargetRotator);

			DrawDebugSphere(GetWorld(), InitLocation, 10.f, 12, FColor::Red, false, 5.f);
			if (WarpComponent)
			{
				WarpComponent->AddOrUpdateWarpTargetFromLocationAndRotation(FName("HangPosition"), TargetLocation, TargetRotator);
			}
			
			// 4-3. Montage 실행
			UAnimMontage* IdleToHangMontage = (ParkourManager->GetLedgeHasFootSurfaceL() || ParkourManager->GetLedgeHasFootSurfaceR()) ? IdleToBracedHang : IdleToFreeHang;
			if (IdleToHangMontage)
			{
				AnimInstance->Montage_Play(IdleToHangMontage);
			}

		}
		else // Ledge에 매달린 상태에서 새로운 Ledge로 점프 (후에 수정 필요)
		{
			LedgeJump(TargetRotator,TargetLocation);
		}
	}
	else // 떨어지는 상태에서 Ledge에 착지 (후에 수정 필요)
	{
		LedgeJump(TargetRotator,TargetLocation);
	}

	
	Movement->SetMovementMode(EMovementMode::MOVE_Flying);

	//HandIKTargetAlpha = 1.0f;
}

/*
* 플레이어가 매달릴 위치 계산 (난간에서 살짝아래위치)
*/
void UParkourHang::CalculateLedgeRotatorAndLocation(FRotator& TargetRotator, FVector& TargetLocation)
{
	FRotator OffsetRotator = UKismetMathLibrary::MakeRotFromX(DetectLedgeNormal);

	float HandOffset = CapsuleHalfHeight + 135.f;
	FVector HeightLocation = DetectLedgeLocation - FVector(0.f, 0.f, HandOffset);

	FVector ForwardDir = UKismetMathLibrary::GetForwardVector(OffsetRotator);
	FVector AwayOffset = ForwardDir * 70.0f;

	TargetLocation = HeightLocation + AwayOffset;

	TargetRotator = OffsetRotator;
	TargetRotator.Pitch = 0.f;
	TargetRotator.Roll = 0.f;
	TargetRotator.Yaw += 180.f;

}

/*
* IdleToHang을 실행할 때 벽에 양 발 지지대가 있는지 확인
*/
void UParkourHang::CheckIfBelowLedgeHasSurface()
{
	USkeletalMeshComponent* Mesh = Player->GetMesh();

	// 1. 왼쪽 발 지지대 확인
	FHitResult HitResultL;
	FVector StartLocationL = Mesh->GetComponentLocation() + FVector(25.f, 0.f, 70.f);
	FVector EndLocationL = StartLocationL + (Player->GetActorForwardVector() * TraceDistanceH);
	bool bHitFootSurfaceL = GetPlayerWorld()->LineTraceSingleByChannel(HitResultL, StartLocationL, EndLocationL, ECollisionChannel::ECC_GameTraceChannel1);
	//DrawDebugLine(GetPlayerWorld(), StartLocationL, EndLocationL, FColor::Red, false, 2.f, 0, 1.f);

	ParkourManager->SetLedgeHasFootSurfaceL(bHitFootSurfaceL);


	// 2. 오른쪽 발 지지대 확인
	FHitResult HitResultR;
	FVector StartLocationR = Mesh->GetComponentLocation() + FVector(-25.f, 0.f, 70.f);
	FVector EndLocationR = StartLocationR + (Player->GetActorForwardVector() * TraceDistanceH);
	bool bHitFootSurfaceR = GetPlayerWorld()->LineTraceSingleByChannel(HitResultR, StartLocationL, EndLocationL, ECollisionChannel::ECC_GameTraceChannel1);
	//DrawDebugLine(GetPlayerWorld(), StartLocationR, EndLocationR, FColor::Red, false, 2.f, 0, 1.f);

	ParkourManager->SetLedgeHasFootSurfaceR(bHitFootSurfaceR);
}

void UParkourHang::FindLedgeHandIKLocation()
{
	USkeletalMeshComponent* Mesh = Player->GetMesh();

	float TraceDistance = 30.f;
	FVector HandOffset = Player->GetActorRightVector() * TraceDistance;


	/* 1. Left Hand IK Location 확인 */
	FHitResult HitResultL;
	FVector StartLocationL = DetectLedgeLocation + FVector(0.f, 0.f, 50.f) + (HandOffset * -1);
	FVector EndLocationL = StartLocationL + FVector(0.f, 0.f, -100.f);
	bool bHitLedgeHandL = GetPlayerWorld()->LineTraceSingleByChannel(HitResultL, StartLocationL, EndLocationL, ECollisionChannel::ECC_GameTraceChannel1);
	DrawDebugLine(GetPlayerWorld(), StartLocationL, EndLocationL, FColor::Red, true, 5.f, 0, 1.f);

	// Check the InitialOverlap.
	if (bHitLedgeHandL && HitResultL.Distance > 0)
	{
		ParkourManager->SetLedgeHasHandSurfaceL(true);
		ParkourManager->SetHandIKLocationL(HitResultL.ImpactPoint);
	}


	/* 2. Right Hand IK Location 확인 */
	FHitResult HitResultR;
	FVector StartLocationR = DetectLedgeLocation + FVector(0.f, 0.f, 50.f) + HandOffset;
	FVector EndLocationR = StartLocationR + FVector(0.f, 0.f, -100.f);
	bool bHitLedgeHandR = GetPlayerWorld()->LineTraceSingleByChannel(HitResultR, StartLocationR, EndLocationR, ECollisionChannel::ECC_GameTraceChannel1);
	DrawDebugLine(GetPlayerWorld(), StartLocationR, EndLocationR, FColor::Red, true, 5.f, 0, 1.f);

	// Check the InitialOverlap.
	if (bHitLedgeHandR && HitResultR.Distance > 0)
	{
		ParkourManager->SetLedgeHasHandSurfaceR(true);
		ParkourManager->SetHandIKLocationR(HitResultR.ImpactPoint);
	}
}

void UParkourHang::LedgeJump(FRotator& TargetRotator, FVector& TargetLocation)
{
	FVector NewTargetLocation = TargetLocation + FVector(0.f, 0.f, CapsuleHalfHeight);

	Player->SetActorLocationAndRotation(NewTargetLocation, TargetRotator);

	Movement->SetMovementMode(EMovementMode::MOVE_Flying);

	ParkourManager->SetIsOnLedge(true);
}

