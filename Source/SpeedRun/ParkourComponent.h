// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ParkourDataAsset.h"
#include "EnvironmentDataAsset.h"
#include "ParkourComponent.generated.h"

class ASpeedRunCharacter;
class UMotionWarpingComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPEEDRUN_API UParkourComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UParkourComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


protected:
	UPROPERTY()
	TObjectPtr<ASpeedRunCharacter> Player;

	UPROPERTY()
	TObjectPtr<UAnimInstance> AnimInstance;

	UPROPERTY()
	TObjectPtr<UMotionWarpingComponent> WarpComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayAction")
	UParkourDataAsset* ActionData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayAction")
	FGameplayTagContainer CurrenEnvironmentTags;


public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataAsset")
	TArray<TObjectPtr<UEnvironmentDataAsset>> EnvironmentDataAssets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataAsset")
	TArray<TObjectPtr<UParkourDataAsset>> ParkourDataAssets;



public:
	UFUNCTION()
	void TryParkourAction();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionWarping")
	float MotionWarpingZOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionWarping")
	float MotionWarpingDistance;


public:
	UFUNCTION()
	void HandleToJump();


protected:
	UFUNCTION()
	FHitResult IsDetectObstacle();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Detect|Tag")
	FGameplayTag Tag_DetectObstacle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detect|Tag")
	FGameplayTag Tag_DetectNone;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detect|Value")
	float DetectZOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detect|Value")
	float DetectDistance;


protected:
	UFUNCTION()
	float GetObstacleGapValue(const FHitResult& DetectHitResult);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detect|Tag")
	FGameplayTag TagCategory_SurfaceGap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detect|Value")
	float WidthTrace_Gap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detect|Value")
	float WidthTrace_Count;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detect|Value")
	float WidthTrace_SphereRadius;


protected:
	UFUNCTION()
	float GetObstacleHeightValue(const FHitResult& DetectHitResult);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detect|Tag")
	FGameplayTag TagCategory_ObstacleHeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detect|Value")
	float HeightTrace_ZOffset;


protected:
	UFUNCTION()
	float GetSurfaceGapValue();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detect|Value")
	float HoleTrace_Height;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detect|Value")
	int32 HoleTrace_Count;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detect|Value")
	float HoleTrace_Gap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detect|Value")
	float HoleTrace_SphereRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detect|Value")
	float HoleTrace_Distance;


protected:
	UFUNCTION()
	void AddNewEnvironmentTag(FGameplayTag TagCategory, float Value);

private:
	UFUNCTION()
	bool SphereTrace(FHitResult& HitResult, FVector Start, FVector End, float Radius);
};
