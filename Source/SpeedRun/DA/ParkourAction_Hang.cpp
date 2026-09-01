// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourAction_Hang.h"
#include "SpeedRunCharacter.h"
#include "ParkourMovementComponent.h"
#include "Components/CapsuleComponent.h"

float UParkourAction_Hang::Evaluate(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const
{
	const FObstacleData& ObsData = EnvData.Obstacle_Data;

	if (!ObsData.bHasFrontLedge) return -1.0f;
	if (Component->IsHanging()) return -1.0f;

	if (EnvData.HitParkourTag == WindowLedgeTag)
	{
		return 100.f;
	}

	EMovementMode CurrentMode = Player->GetCharacterMovement()->MovementMode;

	if (CurrentMode == MOVE_Falling)
	{
		return 150.f;
	}

	if (CurrentMode == MOVE_Walking && ObsData.FrontHeight > MinHeight && ObsData.FrontHeight <= MaxHeight)
	{
		return 100.f;
	}

	return -1.0f;
}

void UParkourAction_Hang::ExecuteAction(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const
{
	const FObstacleData& ObsData = EnvData.Obstacle_Data;

	Player->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	Player->GetCharacterMovement()->Velocity = FVector::ZeroVector;

	Component->ClearAllWarpTargets();

	if (ObsData.bHasFrontLedge)
	{
		FVector TargetLocation = ObsData.FrontLedgeLocation + (ObsData.FrontLedgeNormal * LedgeForwardOffset);
		TargetLocation.Z -= Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2 + LedgeVerticalOffset;
		Component->AddWarpTarget(FName("HangPosition"), TargetLocation, ObsData.FrontLedgeNormal);
	}

	PlaySelectedMontage(Player, EnvData);
}
