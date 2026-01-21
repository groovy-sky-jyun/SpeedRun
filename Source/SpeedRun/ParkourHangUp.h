// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ParkourAction.h"
#include "ParkourHangUp.generated.h"

/**
 * 
 */
UCLASS()
class SPEEDRUN_API UParkourHangUp : public UParkourAction
{
	GENERATED_BODY()

public:
	virtual void Initialize(ASpeedRunCharacter* OwnerPlayer, UParkourManager* ParkourComponent) override;

	virtual bool CheckVisibleToAction() override;

	virtual void OnStart() override;

	virtual void OnEnd() override;


public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
	TObjectPtr<UAnimMontage> MantleMontage;

	UFUNCTION()
	void SetFly();
};
