// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Engine/EngineTypes.h"
#include "ParkourDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FAnimInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TagName")
	FGameplayTag TagName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	class UAnimMontage* AnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	float SpeedRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	FName WarpTargetName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Priority")
	int32 Priority;
};

UCLASS(BlueprintType)
class SPEEDRUN_API UParkourDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Tag")
	FGameplayTag CategoryTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TArray<FAnimInfo> TagList;
};




