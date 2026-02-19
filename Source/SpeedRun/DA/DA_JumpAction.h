// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DA_JumpAction.generated.h"

USTRUCT(BlueprintType)
struct FJumpOption
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Tag")
	FGameplayTag JumpTagName;

	UPROPERTY(EditAnywhere, Category = "Value")
	float ZVelocity;

	UPROPERTY(EditAnywhere, Category = "Value")
	float Impulse;

	UPROPERTY(EditAnywhere, Category = "Value")
	float GravityScale;
};

UCLASS(BlueprintType)
class SPEEDRUN_API UDA_JumpAction : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Tag")
	TArray<FJumpOption> JumpList;
};
