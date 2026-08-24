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

public:
	// 넘을 수 있는 앞면 높이 범위
	UPROPERTY(EditAnywhere, Category = "Vault")
	float MinHeight = 50.f;

	UPROPERTY(EditAnywhere, Category = "Vault")
	float MaxHeight = 150.f;

	// 이보다 두꺼우면 넘지 못한다
	UPROPERTY(EditAnywhere, Category = "Vault")
	float MaxDepth = 200.f;

};
