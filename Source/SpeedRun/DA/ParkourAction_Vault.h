// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ParkourActionBase.h"
#include "ParkourAction_Vault.generated.h"

/**
 * 
 */
UCLASS()
class SPEEDRUN_API UParkourAction_Vault : public UParkourActionBase
{
	GENERATED_BODY()
	
public:
	virtual float Evaluate(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const override;
	virtual void ExecuteAction(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const override;

};
