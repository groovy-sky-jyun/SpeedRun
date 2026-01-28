// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "EnvironmentDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FObstacleState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Tag")
	FGameplayTagContainer EnvironmentTags;

	UPROPERTY(EditAnywhere, Category = "Value")
	float MinHeight;

	UPROPERTY(EditAnywhere, Category = "Value")
	float MaxHeight;

	UPROPERTY(EditAnywhere, Category = "Value")
	float MinWidth;

	UPROPERTY(EditAnywhere, Category = "Value")
	float MaxWidth;

	UPROPERTY(EditAnywhere, Category = "Value")
	float FloorSurfaceHeight; // 착지하는 바닥 높이
};


UCLASS(BlueprintType)
class SPEEDRUN_API UEnvironmentDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// 우선순위 순으로 배치
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
	TArray<FObstacleState> ObstacleRules;
};
