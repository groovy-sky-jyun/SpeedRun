// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "GameplayTagContainer.h"
#include "SpeedRunCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_DYNAMIC_DELEGATE(FOnParkourAnimEndedDelegate);

USTRUCT(BlueprintType)
struct FParkourActionPayload
{
	GENERATED_BODY()

public:
	// 실행할 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* AnimMontage = nullptr;

	// 모션 워핑 타겟 이름 (예: "SlideTarget", "VaultTarget")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TargetName = NAME_None;

	// 워핑할 목표 위치
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TargetLocation = FVector::ZeroVector;

	// 워핑할 목표 회전
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator TargetRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FOnParkourAnimEndedDelegate OnParkourAnimEndedDelegate;

};

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */


UCLASS(abstract)
class ASpeedRunCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Parkour movement */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	//TObjectPtr<class UParkourComponent> ParkourComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UParkourManager> ParkourComponent;

	/** Motion Warping */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UMotionWarpingComponent> MotionWarpingComponent;

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }



public:

	/** Constructor */
	ASpeedRunCharacter();	
	
	UFUNCTION(BlueprintCallable, Category = "Parkour|Anim")
	void ExecuteParkourDelegate(const FOnParkourAnimEndedDelegate& DelegateToCall);

	UFUNCTION()
	UMotionWarpingComponent* GetMotionWarpingComponent();

	//UFUNCTION()
	//UParkourComponent* GetParkourComponent();

	UFUNCTION()
	UParkourManager* GetParkourManager();

protected:

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MouseLookAction;

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:
	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoLook(float Yaw, float Pitch);


public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Parkour|Anim")
	void PlayMotionWarping(FParkourActionPayload ParkourActionPayload);

};

