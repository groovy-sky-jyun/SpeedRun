// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DA/ParkourActionBase.h"
#include "Animation/AnimMontage.h"
#include "ParkourAction_WindowMantle.generated.h"
/**
 * 
 */
UCLASS()
class SPEEDRUN_API UParkourAction_WindowMantle : public UParkourActionBase
{
	GENERATED_BODY()

public:
	virtual float Evaluate(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const override;
	virtual void ExecuteAction(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const override;


public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	FGameplayTag BlockTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	UAnimMontage* ActionMontage;
};
