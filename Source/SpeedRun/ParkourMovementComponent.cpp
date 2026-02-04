// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourMovementComponent.h"
#include "SpeedRunCharacter.h"

UParkourMovementComponent::UParkourMovementComponent()
{
	bOrientRotationToMovement = true;
	RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	JumpZVelocity = 500.f;
	AirControl = 0.35f;
	MaxWalkSpeed = 700.f;
	MinAnalogWalkSpeed = 20.f;
	BrakingDecelerationWalking = 2000.f;
	BrakingDecelerationFalling = 1500.0f;
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
	case CMOVE_NONE:
		//함수 호출
		break;
	case CMOVE_OnLedge:
		//함수 호출
		break;
	case CMOVE_Climb:
		//함수 호출
		break;
	}
}


bool UParkourMovementComponent::DoJump(bool bReplayingMoves, float DeltaTime)
{
	UE_LOG(LogTemp, Warning, TEXT("Custom Jump Start"));
	if (Super::DoJump(bReplayingMoves, DeltaTime))
	{
		
		FVector ForwardDir = Player->GetActorForwardVector();
		
		Velocity.X = ForwardDir.X * JumpForwardImpulse;
		Velocity.Y = ForwardDir.Y * JumpForwardImpulse;
		

		//ResetJumpValues();

		return true;
	}
	return false;
}


void UParkourMovementComponent::SetJumpValues(float Gravity, float ZOffset, float Impulse)
{
	GravityScale = Gravity;
	JumpZVelocity = ZOffset;
	JumpForwardImpulse = Impulse;
}

void UParkourMovementComponent::ResetJumpValues()
{
	JumpZVelocity = 500.f;;
	JumpForwardImpulse = 0.f;
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
