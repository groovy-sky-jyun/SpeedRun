// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

class ACharacter;
class ASpeedRunCharacter;
class UCharacterMovementComponent;
class UParkourComponent;

UCLASS()
class SPEEDRUN_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	

public:
	virtual void NativeInitializeAnimation() override; 

	virtual void NativeUpdateAnimation(float DeltaSeconds);


public:

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	TObjectPtr<ACharacter> Character;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UCharacterMovementComponent> Movement;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<ASpeedRunCharacter> Player;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UParkourComponent> ParkourComponent;


public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	FVector Velocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bShouldMove;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsFalling;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float Direction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour")
	bool bIsHanging;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour")
	float ShimmySpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour|IK")
	FVector HandIKLocationL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour|IK")
	FVector HandIKLocationR;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour|IK")
	FRotator HandIKRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour|IK")
	float HandIKAlpha;

	/*
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bCanMove;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parkour|Ledge")
	bool bIsOnLedge;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour|Ledge")
	bool bLedgeHasFootSurfaceL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour|Ledge")
	bool bLedgeHasFootSurfaceR;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour|Ledge")
	float LeftFootAlpha;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour|Ledge")
	float RightFootAlpha;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour|Ledge")
	bool bOverrideFootIK;

 */


public:
	/*
	UFUNCTION()
	void AnimNotify_ToHangBlendOut();

	UFUNCTION()
	void AnimNotify_ToHangEnd();

	UFUNCTION()
	void AnimNotify_ClimbUpEnd();*/
};
