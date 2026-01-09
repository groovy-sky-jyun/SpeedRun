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


/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */

UCLASS(abstract)
class ASpeedRunCharacter : public ACharacter
{
	GENERATED_BODY()


private:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UParkourManager> ParkourComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UMotionWarpingComponent> MotionWarpingComponent;


public:
	explicit ASpeedRunCharacter(const FObjectInitializer& ObjectInitializer);	


protected:
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* UpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* DownAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* DashAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InteractAction;
	

protected:	/** Called for input */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void HandleLook(const FInputActionValue& Value);

	UFUNCTION()
	void HandleMove(const FInputActionValue& Value);

	UFUNCTION()
	void HandleUp(const FInputActionValue& Value);

	UFUNCTION()
	void HandleDown(const FInputActionValue& Value);

	UFUNCTION()
	void HandleDash(const FInputActionValue& Value);

	UFUNCTION()
	void HandleInteract(const FInputActionValue& Value);


public:
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoLook(float Yaw, float Pitch);

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoMove(float Right, float Forward);

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoUp();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoDown();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoDash();

	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoInteract();



public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintCallable, Category="Components")
	FORCEINLINE class UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }

	UFUNCTION(BlueprintCallable, Category = "Components")
	FORCEINLINE class UParkourManager* GetParkourManager() const { return ParkourComponent; }
};

