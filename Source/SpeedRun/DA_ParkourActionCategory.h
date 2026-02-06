// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DA_ParkourActionCategory.generated.h"

USTRUCT(BlueprintType)
struct FParkourActionList
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Tag")
	FGameplayTag ActionTag;

	UPROPERTY(EditAnywhere, Category = "Tag")
	TArray<FGameplayTagContainer> ConditionTags;
};

UCLASS(BlueprintType)
class SPEEDRUN_API UDA_ParkourActionCategory : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Tag")
	FGameplayTag ActionCategory;

	UPROPERTY(EditAnywhere, Category="Tag")
	TArray<FParkourActionList> ActionList;
};
