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
	Velocity = FVector::ZeroVector;

	Acceleration = FVector::ZeroVector;

	//물리 이동 계산 종료
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
