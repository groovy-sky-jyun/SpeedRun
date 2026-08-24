// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ParkourActionBase.h"
#include "ParkourAction_Hang.generated.h"

/**
 * 
 */
UCLASS()
class SPEEDRUN_API UParkourAction_Hang : public UParkourActionBase
{
	GENERATED_BODY()
	
public:
	virtual float Evaluate(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const override;
	virtual void ExecuteAction(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour")
	FGameplayTag WindowLedgeTag;

	UPROPERTY(EditAnywhere, Category = "Hang")
	float MinHeight = 150.f;

	UPROPERTY(EditAnywhere, Category = "Hang")
	float MaxHeight = 250.f;

	// Ledge 위치 보정
	UPROPERTY(EditAnywhere, Category = "Hang")
	float LedgeForwardOffset = 62.f;

	UPROPERTY(EditAnywhere, Category = "Hang")
	float LedgeVerticalOffset = 26.f;
};
