// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

class ACharacter;
class ASpeedRunCharacter;
class UCharacterMovementComponent;
class UParkourManager;

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
	TObjectPtr<UCharacterMovementComponent> Movement;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<ASpeedRunCharacter> Player;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UParkourManager> ParkourManager;


public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bCanMove;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour")
	bool bIsCrouching;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour|Ledge")
	FVector HandIKLocationL;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour|Ledge")
	FVector HandIKLocationR;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour|Ledge")
	float HandIKTargetAlpha;


public:
	UFUNCTION()
	void AnimNotify_ToHangBlendOut();

	UFUNCTION()
	void AnimNotify_ToHangEnd();



	UFUNCTION()
	void AnimNotify_ClimbUpEnd();
};
