// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourAction_Mantle.h"
#include "SpeedRunCharacter.h"
#include "Components/CapsuleComponent.h"
#include "ParkourMovementComponent.h"
#include "ChooserFunctionLibrary.h"

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

	if (CurrentMode == MOVE_Custom && CustomMode == static_cast<uint8>(ECustomMovementMode::CUSTOM_Hang))
	{
		if (ObsData.Depth >= CapsuleRadius * 2.0f)
		{
			return 200.f;
		}
	}

	if (CurrentMode == MOVE_Walking)
	{
		float MaxHeightVault = 150.f;
		float MaxDepthVault = 200.f;
		float MaxHeightMantle = 250.f;

		if (ObsData.FrontHeight <= MaxHeightVault && ObsData.Depth > MaxDepthVault)
		{
			return 100.0f;
		}

		if (ObsData.FrontHeight > MaxHeightVault && ObsData.FrontHeight <= MaxHeightMantle)
		{
			if (ObsData.Depth >= CapsuleRadius * 2.0f)
			{
				return 100.f;
			}
		}
	}

	return -1.0f;
}

void UParkourAction_Mantle::ExecuteAction(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const
{
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
	FVector MantleTargetLocation = ObsData.UpperSurfaceLocation;

	if (ObsData.bHasFrontLedge)
	{
		Component->AddWarpTarget(FName("FrontEdge"), ObsData.FrontLedgeLocation, ObsData.FrontLedgeNormal);
	}
	if (ObsData.bHasLandingSurface)
	{
		Component->AddWarpTarget(FName("UpperLanding"), MantleTargetLocation, ObsData.FrontLedgeNormal);
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