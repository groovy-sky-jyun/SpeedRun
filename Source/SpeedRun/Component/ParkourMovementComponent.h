// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ParkourMovementComponent.generated.h"

class ASpeedRunCharacter;

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CUSTOM_None,
	CUSTOM_Hang,
	CUSTOM_WallRun,
	CUSTOM_Swing,
	CUSTOM_WallSidle
};
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SPEEDRUN_API UParkourMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UParkourMovementComponent();
	

protected:
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, Category = "Owner")
	TObjectPtr<ASpeedRunCharacter> Player;


protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float MaxClimbSpeed = 250.f;


public:
	UFUNCTION(BlueprintCallable, Category = "Physics")
	float GetSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Physics")
	bool IsWalk() const;

};
