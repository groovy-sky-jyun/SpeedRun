// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourMovementComponent.h"
#include "ParkourComponent.h"
#include "SpeedRunCharacter.h"

UParkourMovementComponent::UParkourMovementComponent()
{
	bOrientRotationToMovement = true;
	RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	MinAnalogWalkSpeed = 20.f;
}

void UParkourMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<ASpeedRunCharacter>(GetOwner());
}

void UParkourMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	switch (CustomMovementMode)
	{
	case CUSTOM_Hang:
		PhysHang(deltaTime, Iterations);
	case CUSTOM_WallRun:
		PhysWallRun(deltaTime, Iterations);
	default:
		break;
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
