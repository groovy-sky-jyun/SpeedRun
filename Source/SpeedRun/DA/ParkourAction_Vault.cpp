// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourAction_Vault.h"
#include "SpeedRunCharacter.h"
#include "ParkourMovementComponent.h"

float UParkourAction_Vault::Evaluate(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const
{
	// MOVE_Walking 상태에서만 Vault 가능
	if (Player->GetCharacterMovement()->MovementMode != MOVE_Walking)
	{
		return -1.0f;
	}

	const FObstacleData& ObsData = EnvData.Obstacle_Data;

	if (ObsData.FrontHeight < MinHeight) return -1.0f;
	if (ObsData.FrontHeight > MaxHeight) return -1.0f;
	if (ObsData.Depth > MaxDepth) return -1.0f;

	return 100.0f;
}

void UParkourAction_Vault::ExecuteAction(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const
{
	const FObstacleData& ObsData = EnvData.Obstacle_Data;

	Component->ClearAllWarpTargets();

	if (ObsData.bHasFrontLedge)
	{
		Component->AddWarpTarget(FName("FrontEdge"), ObsData.FrontLedgeLocation, ObsData.FrontLedgeNormal);
	}
	if (ObsData.bHasLandingSurface)
	{
		Component->AddWarpTarget(FName("Landing"), ObsData.LandingSurfaceLocation, ObsData.FrontLedgeNormal);
	}

	PlaySelectedMontage(Player, EnvData);
}
