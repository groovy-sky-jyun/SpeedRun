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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Hang")
	float MaxShimmySpeed = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Hang")
	float ShimmyInterpSpeed = 10.f;


	virtual void PhysCustom(float deltaTime, int32 Iterations) override;


private:
	void PhysHang(float deltaTime, int32 Iterations);
	void PhysWallRun(float deltaTime, int32 Iterations);

public:
	UFUNCTION(BlueprintCallable, Category = "Physics")
	float GetSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Physics")
	bool IsWalk() const;

};
