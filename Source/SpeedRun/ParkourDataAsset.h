// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Engine/EngineTypes.h"
#include "ParkourDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class SPEEDRUN_API UParkourDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TagName")
	FGameplayTag ActionTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conditions")
	FGameplayTagContainer RequirementTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	class UAnimMontage* AnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	FName WarpTargetName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Priority")
	int32 Priority;
	
};
