// Fill out your copyright notice in the Description page of Project Settings.


#include "DA/ParkourAction_WindowMantle.h"
#include "SpeedRunCharacter.h"
#include "ParkourMovementComponent.h"
#include "ChooserFunctionLibrary.h"
#include "Components/CapsuleComponent.h"

float UParkourAction_WindowMantle::Evaluate(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const
{
	if (EnvData.HitParkourTag == FGameplayTag::EmptyTag) return -1.0f;

	const FObstacleData& ObsData = EnvData.Obstacle_Data;

	if (!ObsData.bHasFrontLedge) return -1.0f;

	UCharacterMovementComponent* MovementComponent = Player->GetCharacterMovement();
	UParkourMovementComponent* ParkourMovement = Player->GetParkourMovement();

	EMovementMode CurrentMode = MovementComponent->MovementMode;
	uint8 CustomMode = ParkourMovement ? ParkourMovement->CustomMovementMode : 0;

	float CapsuleRadius = Player->GetCapsuleComponent()->GetScaledCapsuleRadius();

	if (CurrentMode == MOVE_Custom && CustomMode == static_cast<uint8>(ECustomMovementMode::CUSTOM_Hang))
	{
		return 200.f;
	}

	if (CurrentMode == MOVE_Walking)
	{
		return 100.0f;
	}

	return -1.0f;
}

void UParkourAction_WindowMantle::ExecuteAction(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const
{
	/*
	if (Component->bIsHanging)
	{
		Component->bIsHanging = false;
	}

	Component->ClearAllWarpTargets();

	UCharacterMovementComponent* MovementComponent = Player->GetCharacterMovement();
	if (MovementComponent->CustomMovementMode == static_cast<uint8>(ECustomMovementMode::CUSTOM_Hang))
	{
		MovementComponent->SetMovementMode(MOVE_Flying); // 중력 무시하고 애니메이션 수행
		MovementComponent->Velocity = FVector::ZeroVector;
	}

	const FObstacleData& ObsData = EnvData.Obstacle_Data;
	if (ObsData.bHasFrontLedge)
	{
		FVector TargetLocation = ObsData.FrontLedgeLocation + (ObsData.FrontLedgeNormal * Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		Component->AddWarpTarget(FName("FrontLedge"), ObsData.FrontLedgeLocation, ObsData.FrontLedgeNormal);
	}
	if (ObsData.bHasLandingSurface)
	{
		Component->AddWarpTarget(FName("Landing"), ObsData.LandingSurfaceLocation, ObsData.FrontLedgeNormal);
	}

	if (ActionMontage && Player)
	{
		Player->PlayAnimMontage(ActionMontage);
	}*/
}
