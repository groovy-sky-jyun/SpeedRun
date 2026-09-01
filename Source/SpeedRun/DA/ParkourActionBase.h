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
	

protected:
	// ActionChooser 로 현재 환경에 맞는 몽타주를 골라 재생한다.
	// Chooser 가 없거나 조건에 맞는 행이 없으면 아무것도 하지 않는다.
	void PlaySelectedMontage(ASpeedRunCharacter* Player, const FEnvData& EnvData) const;
};
