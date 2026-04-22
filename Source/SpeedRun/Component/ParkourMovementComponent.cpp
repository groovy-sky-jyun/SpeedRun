// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourMovementComponent.h"
#include "ParkourComponent.h"
#include "SpeedRunCharacter.h"

UParkourMovementComponent::UParkourMovementComponent()
{
	bOrientRotationToMovement = true;
	RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	MinAnalogWalkSpeed = 20.f;
	SetWalkableFloorAngle(MaxWalkableAngle); //50도 이상부터는 걷기 불가능
}

void UParkourMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<ASpeedRunCharacter>(GetOwner());
}

// 각도에 따른 저항이 포함된 속도 구하기
float UParkourMovementComponent::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	if (MovementMode == MOVE_Walking && CurrentFloor.bBlockingHit) //바닥에 닿아 있을 때
	{
		// 플레이어가 가려는 방향
		FVector MoveDirection = Acceleration.GetSafeNormal();
		// 바닥 법선 벡터
		FVector FloorNormal = CurrentFloor.HitResult.ImpactNormal;
		// 음수: 오르막길 / 양수: 내리막길
		float SlopeDot = FVector::DotProduct(MoveDirection, FloorNormal);

		if (SlopeDot < 0.f)
		{
			// 바닥 각도 계산
			float CurrentAngle = FMath::RadiansToDegrees(FMath::Acos(FloorNormal.Z));
			// 각도에 따라 1.0 ~ 0.3 으로 조절
			float SpeedMultiplier = FMath::GetMappedRangeValueClamped(
				FVector2D(0.f, MaxWalkableAngle), // 각도 범위
				FVector2D(1.f, 0.05f), // 출력 범위
				CurrentAngle //실제 바닥 각도
			);
			MaxSpeed *= SpeedMultiplier;
		}
	}
	return MaxSpeed;
}



void UParkourMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	switch (CustomMovementMode)
	{
	case CUSTOM_Hang:
		PhysHang(deltaTime, Iterations);
		break;
	default:
		return;
	}
}

void UParkourMovementComponent::PhysHang(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME) return;

	FVector WallNormal = UpdatedComponent->GetForwardVector() * -1.f;
	FVector WallRight = FVector::CrossProduct(FVector::UpVector, WallNormal).GetSafeNormal();

	// 입력값과 WallRight 일치하는지 확인 -> 키 입력의 왼쪽/오른쪽 구분
	float InputDir = FVector::DotProduct(Acceleration.GetSafeNormal(), WallRight);

	if (FMath::Abs(InputDir) > 0.1f)
	{
		Velocity = WallRight * (InputDir > 0 ? MaxShimmySpeed : -MaxShimmySpeed);
	}
	else
	{
		Velocity = FMath::VInterpTo(Velocity, FVector::ZeroVector, deltaTime, 60.f);
	}

	Velocity.Z = 0.f;

	FVector Adjusted = Velocity * deltaTime;
	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);

	UpdateComponentVelocity();
}

void UParkourMovementComponent::PhysWallRun(float deltaTime, int32 Iterations)
{
}

float UParkourMovementComponent::GetSpeed() const
{
	return Velocity.Length();
}

bool UParkourMovementComponent::IsWalk() const
{
	if (!IsWalking() || FMath::IsNearlyZero(GetSpeed()))
	{
		return false;
	}

	return true;
}
