// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "ParkourComponent.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<ACharacter>(GetOwningActor());
	if (Character)
	{
		MovementComponent = Character->GetCharacterMovement();
	}
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (Character == nullptr || MovementComponent == nullptr) return;


	/*
	* Set velocity and ground speed from the movement components velocity. 
	* Ground speed is calculated from only the X and Y axis of the velocity, 
	* so moving up or down does not affect it.
	*/
	Velocity = MovementComponent->Velocity;
	GroundSpeed = Velocity.Size2D();


	/*
	* Set Should Move to true only if ground speed is above a small threshold 
	* (to prevent incredibly small velocities from triggering animations) 
	* and if there is currently acceleration (input) applied.
	*
	* izeSquared2D(): x^2 + y^2 (성능 최적화를 위해 제곱을 사용)
	* 가속도가 0이 아니고 속도가 0.01보다 큰 경우
	*/
	bShouldMove = (MovementComponent->GetCurrentAcceleration().SizeSquared2D() != 0.0f) && (GroundSpeed > 0.01);


	/*
	* Set Is Falling from the movement components falling state.
	*/
	bIsFalling = MovementComponent->IsFalling();


	/*
	* Calculate direction using the delta between the velocity and the actor rotation. 
	* When the character is not strafing, 
	* clamp the value between - and + 45 degrees so that backwards animations do not play when turning around, 
	* but running into wall looks better.
	*/
	float CalculatedDirection = UKismetAnimationLibrary::CalculateDirection(Velocity, Character->GetActorRotation());
	float ClampCalculatedDirection = FMath::Clamp(CalculatedDirection, -45.0f, 45.0f);
	Direction = MovementComponent->bOrientRotationToMovement ? ClampCalculatedDirection : CalculatedDirection;


	/*
	* 
	*/
	bIsCrouching = MovementComponent->IsCrouching();

	ASpeedRunCharacter* Player = Cast<ASpeedRunCharacter>(Character);
	UParkourComponent* ParkourComponent = Player->GetParkourComponent();

	bIsOnLedge = ParkourComponent->GetIsOnLedge();
	bBelowLedgeHasSurfaceL = ParkourComponent->GetBelowLedgeHasSurfaceL();
	bBelowLedgeHasSurfaceR = ParkourComponent->GetBelowLedgeHasSurfaceR();

	// FInterpTo( T1  Current, T2 Target, T3 DeltaTime, T4 InterpSpeed )
	LeftFootAlpha = FMath::FInterpTo(LeftFootAlpha, float(!bBelowLedgeHasSurfaceL), GetWorld()->GetDeltaSeconds(), 0.2f);

	RightFootAlpha = FMath::FInterpTo(RightFootAlpha, float(!bBelowLedgeHasSurfaceR), GetWorld()->GetDeltaSeconds(), 0.2f);
}

void UPlayerAnimInstance::AnimNotify_ToHangEnd()
{
	ASpeedRunCharacter* Player = Cast<ASpeedRunCharacter>(Character);
	UParkourComponent* ParkourComponent = Player->GetParkourComponent();
	ParkourComponent->SetIsOnLedge(true);
	bIsOnLedge = true;

	if (UAnimMontage* CurrentMontage = GetCurrentActiveMontage())
	{
		Montage_Stop(0.2f, CurrentMontage);
	}

	
}
