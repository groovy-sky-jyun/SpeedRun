// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ParkourComponent.h"
#include "ParkourActionBase.generated.h"

class ASpeedRunCharacter;
class UParkourComponent;
class UChooserTable;

UCLASS(Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced, Abstract)
class SPEEDRUN_API UParkourActionBase : public UDataAsset
{
	GENERATED_BODY()

public:
	virtual float Evaluate(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const;
	virtual void ExecuteAction(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const;
	

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parkour")
	UChooserTable* ActionChooser;
	
};
