// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ParkourAction.generated.h"

class ASpeedRunCharacter;
class UCharacterMovementComponent;
class UCapsuleComponent;
class UParkourManager;
class UMotionWarpingComponent;
class IParkourInputType;

UENUM(BlueprintType)
enum class EParkourStateType : uint8
{
	None,
	Up,
	Down,
	Sprint,
	Interact
};


UCLASS(Abstract, EditInlineNew, Blueprintable)
class SPEEDRUN_API UParkourAction : public UObject
{
	GENERATED_BODY()
	

protected:
	UPROPERTY(EditDefaultsOnly, Category = "ActionType")
	EParkourStateType InputType = EParkourStateType::None;

	UPROPERTY()
	TObjectPtr<ASpeedRunCharacter> Player;

	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> Movement;

	UPROPERTY()
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY()
	TObjectPtr<UParkourManager> ParkourManager;

	UPROPERTY()
	TObjectPtr<UMotionWarpingComponent> WarpComponent;

	float CapsuleHalfHeight;

	float DefaultCrouchedHalfHeight;

public:
	virtual void Initialize(ASpeedRunCharacter* OwnerPlayer, UParkourManager* ParkourComponent);

	const UWorld* GetPlayerWorld();

	virtual bool CheckVisibleToAction() { return false; }

	virtual void OnStart() {};

	virtual void OnUpdate() {};

	virtual void OnEnd() {};

	EParkourStateType GetInputType() { return InputType; }
};
