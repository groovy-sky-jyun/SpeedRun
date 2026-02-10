// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DA_TraceOptions.h"
#include "DA_BoxTraceOption.generated.h"


UCLASS(BlueprintType)
class SPEEDRUN_API UDA_BoxTraceOption : public UDA_TraceOptions
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Config")
	float BoxHalfSize;
};


