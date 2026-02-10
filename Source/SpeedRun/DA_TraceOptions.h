// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DA_TraceOptions.generated.h"

/**
 * 
 */
UCLASS()
class SPEEDRUN_API UDA_TraceOptions : public UDataAsset
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, Category = "Tag")
	FGameplayTag CategoryTag;

	UPROPERTY(EditAnywhere, Category = "Config")
	float FrontOffset;

	UPROPERTY(EditAnywhere, Category = "Config")
	float ZOffset;

	UPROPERTY(EditAnywhere, Category = "Config")
	float Distance;
};
