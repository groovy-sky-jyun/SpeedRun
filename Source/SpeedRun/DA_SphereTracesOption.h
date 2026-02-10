// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DA_TraceOptions.h"
#include "DA_SphereTracesOption.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class SPEEDRUN_API UDA_SphereTracesOption : public UDA_TraceOptions
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Config")
	int32 Count;

	UPROPERTY(EditAnywhere, Category = "Config")
	float Gap;

	UPROPERTY(EditAnywhere, Category = "Config")
	float Radius;
};
