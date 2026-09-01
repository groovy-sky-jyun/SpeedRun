// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourAction_Mantle.h"
#include "SpeedRunCharacter.h"
#include "Components/CapsuleComponent.h"
#include "ParkourMovementComponent.h"

float UParkourAction_Mantle::Evaluate(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const
{
	const FObstacleData& ObsData = EnvData.Obstacle_Data;

	if (!ObsData.bHasUpperSurface)
	{
		return -1.0f;
	}

	UCharacterMovementComponent* MovementComponent = Player->GetCharacterMovement();
	UParkourMovementComponent* ParkourMovement = Player->GetParkourMovement();

	EMovementMode CurrentMode = MovementComponent->MovementMode;
	uint8 CustomMode = ParkourMovement ? ParkourMovement->CustomMovementMode : 0;

	float CapsuleRadius = Player->GetCapsuleComponent()->GetScaledCapsuleRadius();
	float StandableDepth = CapsuleRadius * MinDepth;

	if (CurrentMode == MOVE_Custom && CustomMode == static_cast<uint8>(ECustomMovementMode::CUSTOM_Hang))
	{
		if (ObsData.Depth >= StandableDepth)
		{
			return 200.f;
		}
	}

	if (CurrentMode == MOVE_Walking)
	{
		if (ObsData.FrontHeight <= LowObstacleMaxHeight && ObsData.Depth > VaultMaxDepth)
		{
			return 100.0f;
		}

		if (ObsData.FrontHeight > LowObstacleMaxHeight && ObsData.FrontHeight <= MaxHeight)
		{
			if (ObsData.Depth >= StandableDepth)
			{
				return 100.f;
			}
		}
	}

	return -1.0f;
}

void UParkourAction_Mantle::ExecuteAction(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const
{
	Component->ClearAllWarpTargets();

	UCharacterMovementComponent* MovementComponent = Player->GetCharacterMovement();
	if (MovementComponent->CustomMovementMode == static_cast<uint8>(ECustomMovementMode::CUSTOM_Hang))
	{
		MovementComponent->SetMovementMode(MOVE_Flying); // 중력 무시하고 애니메이션 수행
		MovementComponent->Velocity = FVector::ZeroVector;
	}

	const FObstacleData& ObsData = EnvData.Obstacle_Data;
	FVector MantleTargetLocation = ObsData.UpperSurfaceLocation;

	if (ObsData.bHasFrontLedge)
	{
		Component->AddWarpTarget(FName("FrontEdge"), ObsData.FrontLedgeLocation, ObsData.FrontLedgeNormal);
	}
	if (ObsData.bHasLandingSurface)
	{
		Component->AddWarpTarget(FName("UpperLanding"), MantleTargetLocation, ObsData.FrontLedgeNormal);
	}

	PlaySelectedMontage(Player, EnvData);
}