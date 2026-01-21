// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SpeedRunCharacter.h"
#include "ParkourAction.generated.h"

//class UCharacterMovementComponent;
class UParkourMovementComponent;
class UCapsuleComponent;
class UParkourManager;
class UMotionWarpingComponent;
class IParkourInputType;

USTRUCT(BlueprintType)
struct FDetectWallInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	bool bHit = false;

	UPROPERTY(BlueprintReadOnly)
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	FVector HitNormal = FVector::ZeroVector;
};

UCLASS(Abstract, EditInlineNew, Blueprintable)
class SPEEDRUN_API UParkourAction : public UObject
{
	GENERATED_BODY()
	

protected:
	UPROPERTY()
	TObjectPtr<ASpeedRunCharacter> Player;

	UPROPERTY()
	TObjectPtr<UParkourMovementComponent> Movement;

	UPROPERTY()
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY()
	TObjectPtr<UParkourManager> ParkourManager;

	UPROPERTY()
	TObjectPtr<UMotionWarpingComponent> WarpComponent;

	UPROPERTY()
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	UPROPERTY()
	TObjectPtr<UAnimInstance> AnimInstance;


	float CapsuleHalfHeight;

	float DefaultCrouchedHalfHeight;


public:
	virtual void Initialize(ASpeedRunCharacter* OwnerPlayer, UParkourManager* ParkourComponent);

	virtual bool CheckVisibleToAction() { return false; }

	virtual void OnStart() {};

	virtual void OnUpdate() {};

	virtual void OnEnd() {};



public:
	UFUNCTION()
	FVector MoveVectorUpward(FVector Vector, float ZOffset);

	UFUNCTION()
	FVector MoveVectorDownward(FVector Vector, float ZOffset);

	UFUNCTION()
	FVector MoveVectorForward(FVector Vector, FRotator Rotation, float Distance);

	UFUNCTION()
	FVector MoveVectorBackward(FVector Vector, FRotator Rotation, float Distance);

	UFUNCTION()
	FVector MoveVectorLeft(FVector Vector, FRotator Rotation, float Distance);

	UFUNCTION()
	FVector MoveVectorRight(FVector Vector, FRotator Rotation, float Distance);
	
	UFUNCTION()
	FRotator ReverseNormal(FVector Normal);
	
	UFUNCTION()
	FDetectWallInfo DetectWall();


public:
	const UWorld* GetPlayerWorld();
};
