// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourAction_Vault.h"
#include "SpeedRunCharacter.h"
#include "ParkourMovementComponent.h"
#include "ChooserFunctionLibrary.h"

float UParkourAction_Vault::Evaluate(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const
{
	// MOVE_Walking 상태에서만 Vault 가능
	if (Player->GetCharacterMovement()->MovementMode != MOVE_Walking)
	{
		return -1.0f;
	}

	const FObstacleData& ObsData = EnvData.Obstacle_Data;

	float MinHeightVault = 50.f;
	float MaxHeightVault = 150.f;
	float MaxDepthVault = 200.0f;

	if (ObsData.FrontHeight < MinHeightVault) return -1.0f;
	if (ObsData.FrontHeight > MaxHeightVault) return -1.0f;
	if (ObsData.Depth > MaxDepthVault) return -1.0f;

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

	if (ActionChooser)
	{
		FChooserEvaluationContext Context;

		FEnvData TempEnvData = EnvData;
		Context.AddStructParam(TempEnvData);

		FInstancedStruct ChooserStruct = UChooserFunctionLibrary::MakeEvaluateChooser(ActionChooser);

		UObject* ResultObj = UChooserFunctionLibrary::EvaluateObjectChooserBase(
			Context,
			ChooserStruct,
			UAnimMontage::StaticClass()
		);

		UAnimMontage* SelectedMontage = Cast<UAnimMontage>(ResultObj);

		if (SelectedMontage)
		{
			Player->PlayAnimMontage(SelectedMontage);
		}
	}
}
