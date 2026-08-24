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

	float CalculatedDirection = UKismetAnimationLibrary::CalculateDirection(Velocity, Character->GetActorRotation());
	float ClampCalculatedDirection = FMath::Clamp(CalculatedDirection, -45.0f, 45.0f);
	Direction = Movement->bOrientRotationToMovement ? ClampCalculatedDirection : CalculatedDirection;

	if (ParkourComponent)
	{
		bIsHanging = ParkourComponent->bIsHanging;

		if (bIsHanging)
		{
			// 내적 +-로 방향 추출
			ShimmySpeed = FVector::DotProduct(Velocity, Character->GetActorRightVector());

			const FEnvData& EnvData = ParkourComponent->CurrentEnvData;
			FVector LedgeLocation = EnvData.Obstacle_Data.FrontLedgeLocation;
			FVector LedgeNormal = EnvData.Obstacle_Data.FrontLedgeNormal;

			FVector WallRight = FVector::CrossProduct(LedgeNormal, FVector::UpVector).GetSafeNormal();

			float HandSpread = 50.f; //양 손 간격

			HandIKLocationL = LedgeLocation - (WallRight * HandSpread);
			HandIKLocationR = LedgeLocation + (WallRight * HandSpread);

			HandIKRotation = LedgeNormal.Rotation();

			HandIKAlpha = FMath::FInterpTo(HandIKAlpha, 1.f, DeltaSeconds, 25.f);
		}
		else
		{
			ShimmySpeed = 0.f;
			HandIKAlpha = FMath::FInterpTo(HandIKAlpha, 0.f, DeltaSeconds, 20.f);
		}
	}

	//bOverrideFootIK = ParkourComponent->GetOverrideFootIK();
	//bLedgeHasFootSurfaceL = ParkourComponent->GetLedgeHasFootSurfaceL();
	//bLedgeHasFootSurfaceR = ParkourComponent->GetLedgeHasFootSurfaceR();
	//LeftFootAlpha = FMath::FInterpTo(LeftFootAlpha, float(!bLedgeHasFootSurfaceL), GetWorld()->GetDeltaSeconds(), 0.2f);
	//RightFootAlpha = FMath::FInterpTo(RightFootAlpha, float(!bLedgeHasFootSurfaceR), GetWorld()->GetDeltaSeconds(), 0.2f);
}