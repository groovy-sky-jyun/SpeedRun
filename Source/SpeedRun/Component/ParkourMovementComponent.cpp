// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourMovementComponent.h"
#include "ParkourComponent.h"
#include "SpeedRunCharacter.h"

UParkourMovementComponent::UParkourMovementComponent()
{
	bOrientRotationToMovement = true;
	RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	AirControl = 0.35f;
	MaxWalkSpeed = 700.f;
	MinAnalogWalkSpeed = 20.f;
}

void UParkourMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<ASpeedRunCharacter>(GetOwner());
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
