// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ParkourHang.h"
#include "ParkourShimmy.generated.h"

/**
 * 
 */
UCLASS()
class SPEEDRUN_API UParkourShimmy : public UParkourHang
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(ASpeedRunCharacter* OwnerPlayer, UParkourManager* ParkourComponent) override;

	bool CheckVisibleToAction(FVector WorldDirection, float ScaleValue);

	virtual void OnStart() override;

	virtual void OnUpdate() override;

	virtual void OnEnd() override;



private:
	bool bIsShimmy;


public:
	bool IsShimmy();

};
