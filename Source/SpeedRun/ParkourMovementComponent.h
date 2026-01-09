// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ParkourMovementComponent.generated.h"

class ASpeedRunCharacter;

UENUM(BlueprintType)
enum EParkourCustomMode
{
	CMOVE_NONE UMETA(DidplayName = "None"),
	CMOVE_OnLedge UMETA(DidplayName = "Move On Ledge"), 
	CMOVE_Climb UMETA(DidplayName = "Climb")  
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SPEEDRUN_API UParkourMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UParkourMovementComponent();
	

protected:
	virtual void BeginPlay() override;

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	UPROPERTY(BlueprintReadWrite, Category = "Owner")
	TObjectPtr<ASpeedRunCharacter> Player;


protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Parkour")
	float MaxClimbSpeed = 250.f;


public:
	UFUNCTION(BlueprintCallable, Category = "Physics")
	float GetSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Physics")
	bool IsWalk() const;
};
