// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ParkourDataAsset.h"
#include "JumpDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class SPEEDRUN_API UJumpDataAsset : public UParkourDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category="JumpPhysics")
	float ZVelocity = 960.f;

	UPROPERTY(EditAnywhere, Category = "JumpPhysics")
	float JumpImpulse = 450.f;

	UPROPERTY(EditAnywhere, Category = "JumpPhysics")
	float GravityScale = 3.5f;
};
