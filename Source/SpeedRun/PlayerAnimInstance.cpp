// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "ParkourManager.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<ACharacter>(GetOwningActor());
	Player = Cast<ASpeedRunCharacter>(Character);
	if (Player)
	{
		Movement = Character->GetCharacterMovement();
		ParkourManager = Player->GetParkourManager();
	}
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (Character == nullptr || Movement == nullptr) return;


	/*
	* Set velocity and ground speed from the movement components velocity. 
	* Ground speed is calculated from only the X and Y axis of the velocity, 
	* so moving up or down does not affect it.
	*/
	Velocity = Movement->Velocity;
	GroundSpeed = Velocity.Size2D();


	/*
	* Set Should Move to true only if ground speed is above a small threshold 
	* (to prevent incredibly small velocities from triggering animations) 
	* and if there is currently acceleration (input) applied.
	*
	* izeSquared2D(): x^2 + y^2 (성능 최적화를 위해 제곱을 사용)
	* 가속도가 0이 아니고 속도가 0.01보다 큰 경우
	*/
	bShouldMove = (Movement->GetCurrentAcceleration().SizeSquared2D() != 0.0f) && (GroundSpeed > 0.01);


	/*
	* Set Is Falling from the movement components falling state.
	*/
	bIsFalling = Movement->IsFalling();


	/*
	* Calculate direction using the delta between the velocity and the actor rotation. 
	* When the character is not strafing, 
	* clamp the value between - and + 45 degrees so that backwards animations do not play when turning around, 
	* but running into wall looks better.
	*/
	float CalculatedDirection = UKismetAnimationLibrary::CalculateDirection(Velocity, Character->GetActorRotation());
	float ClampCalculatedDirection = FMath::Clamp(CalculatedDirection, -45.0f, 45.0f);
	Direction = Movement->bOrientRotationToMovement ? ClampCalculatedDirection : CalculatedDirection;
	
	bIsCrouching = Movement->IsCrouching();

	bIsOnLedge = ParkourManager->GetIsOnLedge();
	bCanMove = ParkourManager->GetCanMove();

	bOverrideFootIK = ParkourManager->GetOverrideFootIK();
	bLedgeHasFootSurfaceL = ParkourManager->GetLedgeHasFootSurfaceL();
	bLedgeHasFootSurfaceR = ParkourManager->GetLedgeHasFootSurfaceR();
	LeftFootAlpha = FMath::FInterpTo(LeftFootAlpha, float(!bLedgeHasFootSurfaceL), GetWorld()->GetDeltaSeconds(), 0.2f);
	RightFootAlpha = FMath::FInterpTo(RightFootAlpha, float(!bLedgeHasFootSurfaceR), GetWorld()->GetDeltaSeconds(), 0.2f);
	

	HandIKLocationL = ParkourManager->GetHandIKLocationL();
	HandIKLocationR = ParkourManager->GetHandIKLocationR();
	bLedgeHasHandSurfaceL = ParkourManager->GetLedgeHasHandSurfaceL();
	bLedgeHasHandSurfaceR = ParkourManager->GetLedgeHasHandSurfaceR();
}


void UPlayerAnimInstance::AnimNotify_ToHangBlendOut()
{
	ParkourManager->SetIsOnLedge(true);
}

void UPlayerAnimInstance::AnimNotify_ToHangEnd()
{
	if (UAnimMontage* CurrentMontage = GetCurrentActiveMontage())
	{
		Movement->StopMovementImmediately();
		Movement->SetMovementMode(EMovementMode::MOVE_None);
	}
}

void UPlayerAnimInstance::AnimNotify_ClimbUpEnd()
{
	/*
	ASpeedRunCharacter* Player = Cast<ASpeedRunCharacter>(Character);
	Player->SetActorEnableCollision(true);

	Player->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);

	//Player->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

	Player->GetParkourManager()->SetCanMove(true);
	bCanMove = true;
	*/
}
