// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ParkourAction.h"
#include "ParkourSprint.generated.h"

/**
 * 
 */
UCLASS()
class SPEEDRUN_API UParkourSprint : public UParkourAction
{
	GENERATED_BODY()
	

public:
	virtual void Initialize(ASpeedRunCharacter* OwnerPlayer, UParkourManager* ParkourComponent) override;

	virtual bool CheckVisibleToAction() override;

	virtual void OnStart() override;

	virtual void OnUpdate() override;

	virtual void OnEnd() override;


protected:
	UPROPERTY(EditAnywhere, Category = "Dash")
	float DashDistance = 2000.f;



protected:
	UPROPERTY(EditDefaultsOnly, Category = "AnimMontage")
	TObjectPtr<UAnimMontage> SprintMontage;

};
