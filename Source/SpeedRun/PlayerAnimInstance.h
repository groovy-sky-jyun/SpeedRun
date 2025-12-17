// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

class ACharacter;
class UCharacterMovementComponent;

UCLASS()
class SPEEDRUN_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	

public:
	// the below functions are the native overrides for each phase
	// Native initialization override point
	virtual void NativeInitializeAnimation() override; 

	// Native update override point. It is usually a good idea to simply gather data in this step and 
	// for the bulk of the work to be done in NativeThreadSafeUpdateAnimation.
	virtual void NativeUpdateAnimation(float DeltaSeconds);
	
	// Native thread safe update override point. Executed on a worker thread just prior to graph update 
	// for linked anim instances, only called when the hosting node(s) are relevant
	//virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds);


public:

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	TObjectPtr<ACharacter> Character;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UCharacterMovementComponent> MovementComponent;

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement")
	FVector Velocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bShouldMove;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsFalling;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float Direction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsCrouching;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ledge")
	bool bIsOnLedge;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ledge")
	bool bBelowLedgeHasSurfaceL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ledge")
	bool bBelowLedgeHasSurfaceR;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ledge")
	float LeftFootAlpha;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ledge")
	float RightFootAlpha;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ledge")
	float OverrideFootIK;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ledge")
	bool bOverrideFootIK;

public:
	UFUNCTION()
	void AnimNotify_ToHangEnd();

	UFUNCTION()
	void AnimNotify_ClimbUpEnd();
};
