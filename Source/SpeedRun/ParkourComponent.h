// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ParkourDataAsset.h"
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

	UFUNCTION()
	void TryParkourAction();



protected:
	UPROPERTY()
	TObjectPtr<ASpeedRunCharacter> Player;

	UPROPERTY()
	TObjectPtr<UAnimInstance> AnimInstance;

	UPROPERTY()
	TObjectPtr<UMotionWarpingComponent> WarpComponent;



public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|DA")
	UParkourDataAsset* ActionData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|Location")
	float ZOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|Location")
	float Distance;

};
