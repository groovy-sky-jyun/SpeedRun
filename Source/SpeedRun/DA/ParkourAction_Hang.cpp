// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourAction_Hang.h"
#include "SpeedRunCharacter.h"
#include "ParkourMovementComponent.h"
#include "ChooserFunctionLibrary.h"
#include "Components/CapsuleComponent.h"

float UParkourAction_Hang::Evaluate(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const
{
	const FObstacleData& ObsData = EnvData.Obstacle_Data;

	if (!ObsData.bHasFrontLedge) return -1.0f;
	if (Component->bIsHanging) return -1.0f;

	EMovementMode CurrentMode = Player->GetCharacterMovement()->MovementMode;

	if (CurrentMode == MOVE_Falling)
	{
		return 150.f;
	}

	float MaxHeightVault = 150.f;
	float MaxHeightHang = 250.f;

	if (CurrentMode == MOVE_Walking && ObsData.FrontHeight > MaxHeightVault && ObsData.FrontHeight <= MaxHeightHang)
	{
		return 100.f;
	}

	return -1.0f;
}

void UParkourAction_Hang::ExecuteAction(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const
{
	const FObstacleData& ObsData = EnvData.Obstacle_Data;

	//Component->AlignToLedge(EnvData);

	Player->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	Player->GetCharacterMovement()->Velocity = FVector::ZeroVector;

	Component->ClearAllWarpTargets();

	if (ObsData.bHasFrontLedge)
	{
		FVector TargetLocation = ObsData.FrontLedgeLocation + (ObsData.FrontLedgeNormal * 62.f);
		TargetLocation.Z -= Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2 + 26.f;
		Component->AddWarpTarget(FName("HangPosition"), TargetLocation, ObsData.FrontLedgeNormal);
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
