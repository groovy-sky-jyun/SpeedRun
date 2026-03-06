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
	virtual bool DoJump(bool bReplayingMoves, float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, Category = "Owner")
	TObjectPtr<ASpeedRunCharacter> Player;


protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float MaxClimbSpeed = 250.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Jump")
	float JumpForwardImpulse = 2.f;


public:
	UFUNCTION()
	void SetJumpValues(float Gravity, float ZOffset, float Impulse);

	UFUNCTION()
	void ResetJumpValues();

	UFUNCTION(BlueprintCallable, Category = "Physics")
	float GetSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Physics")
	bool IsWalk() const;

};
