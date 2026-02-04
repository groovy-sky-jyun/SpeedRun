// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "EnvironmentDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FEnvironmentState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Tag")
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, Category = "Range")
	float MinValue;

	UPROPERTY(EditAnywhere, Category = "Range")
	float MaxValue;

	UPROPERTY(EditAnywhere, Category = "Priority")
	int32 Priority;
};


UCLASS(BlueprintType)
class SPEEDRUN_API UEnvironmentDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Tag")
	FGameplayTag CategoryTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
	TArray<FEnvironmentState> TagList;
};
