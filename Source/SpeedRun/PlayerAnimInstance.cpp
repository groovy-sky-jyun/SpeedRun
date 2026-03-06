// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"
#include "GameFramework/Character.h"
#include "SpeedRunCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "ParkourComponent.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<ACharacter>(GetOwningActor());
	Player = Cast<ASpeedRunCharacter>(Character);
	if (Player)
	{
		Movement = Character->GetCharacterMovement();
		ParkourComponent = Player->GetParkourComponent(); 
	}
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (Character == nullptr || Movement == nullptr) return;

	Velocity = Movement->Velocity;
	GroundSpeed = Velocity.Size2D();

	/*
	* izeSquared2D(): x^2 + y^2 (성능 최적화를 위해 제곱을 사용)
	* 가속도가 0이 아니고 속도가 0.01보다 큰 경우
	*/
	bShouldMove = (Movement->GetCurrentAcceleration().SizeSquared2D() != 0.0f) && (GroundSpeed > 0.01);

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


	//bCanMove = ParkourComponent->GetCanMove();

	//bOverrideFootIK = ParkourComponent->GetOverrideFootIK();
	//bLedgeHasFootSurfaceL = ParkourComponent->GetLedgeHasFootSurfaceL();
	//bLedgeHasFootSurfaceR = ParkourComponent->GetLedgeHasFootSurfaceR();
	//LeftFootAlpha = FMath::FInterpTo(LeftFootAlpha, float(!bLedgeHasFootSurfaceL), GetWorld()->GetDeltaSeconds(), 0.2f);
	//RightFootAlpha = FMath::FInterpTo(RightFootAlpha, float(!bLedgeHasFootSurfaceR), GetWorld()->GetDeltaSeconds(), 0.2f);
	

	//HandIKLocationL = ParkourComponent->GetHandIKLocationL();
	//HandIKLocationR = ParkourComponent->GetHandIKLocationR();
	//bLedgeHasHandSurfaceL = ParkourComponent->GetLedgeHasHandSurfaceL();
	//bLedgeHasHandSurfaceR = ParkourComponent->GetLedgeHasHandSurfaceR();
}

/*
void UPlayerAnimInstance::AnimNotify_ToHangBlendOut()
{
	ParkourComponent->SetIsOnLedge(true);
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
	ParkourComponent->OnEndParkourAction();

	Player->SetActorEnableCollision(true);
	Movement->SetMovementMode(EMovementMode::MOVE_Walking);

	//Player->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

	Player->GetParkourComponent()->SetCanMove(true);
	bCanMove = true;
	
}*/
