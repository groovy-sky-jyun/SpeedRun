// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpeedRunCharacter.h"
#include "EnhancedInputComponent.h"
#include "ParkourAction.h"
#include "ParkourManager.generated.h"


class ASpeedRunPlayerController;
class UInputAction;
struct FInputActionValue;



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPEEDRUN_API UParkourManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	UParkourManager();


protected:
	virtual void BeginPlay() override;


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Owner")
	TObjectPtr<ASpeedRunCharacter> Player;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Owner")
	TObjectPtr<ASpeedRunPlayerController> PlayerController;


public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetupParkourInputComponent(class UEnhancedInputComponent* EnhancedInputComponent);


protected:
	UPROPERTY(EditAnywhere, Category = "Setting|Input")
	UInputAction* LedgeHangUpAction;

	UPROPERTY(EditAnywhere, Category = "Setting|Input")
	UInputAction* LedgeShimmyAction;

	UPROPERTY(EditAnywhere, Category = "Setting|Input")
	UInputAction* LedgeDropAction;

	UPROPERTY(EditAnywhere, Category = "Setting|Input")
	UInputAction* ParkourJumpAction;



public: /* Called when the Player State Changes for input key */
	UFUNCTION(BlueprintCallable, Category = "InputActions")
	void HandleLedgeHangUp(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "InputActions")
	void HandleLedgeShimmy(const FInputActionValue& Value);


	UFUNCTION(BlueprintCallable, Category = "InputActions")
	void HandleLedgeDrop(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "InputActions")
	void HandleParkourJump(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "InputActions")
	bool CanParkourJump(const FInputActionValue& Value);


private:
	bool bCanMove = false;

	bool bIsCrouch = false;

	bool bIsOnLedge = false;

	bool bLedgeHasFootSurfaceL = false;

	bool bLedgeHasFootSurfaceR = false;

	bool bOverrideFootIK = false;

	FVector HandIKLocationR;

	FVector HandIKLocationL;

	bool bLedgeHasHandSurfaceL = false;

	bool bLedgeHasHandSurfaceR = false;

	bool bOverrideHandIK = false;


public:
	/* State */
	FORCEINLINE bool GetCanMove() { return bCanMove; }
	FORCEINLINE void SetCanMove(bool Value)  { bCanMove = Value; }

	FORCEINLINE bool GetIsCrouch() { return bIsCrouch; }
	FORCEINLINE void SetIsCrouch(bool Value) { bIsCrouch = Value; }

	FORCEINLINE bool GetIsOnLedge() { return bIsOnLedge; }
	FORCEINLINE void SetIsOnLedge(bool Value) { bIsOnLedge = Value; }


	/* FootIK */
	FORCEINLINE bool GetOverrideFootIK() { return bOverrideFootIK; }
	FORCEINLINE void SetOverrideFootIK(bool Value) { bOverrideFootIK = Value; }

	FORCEINLINE bool GetLedgeHasFootSurfaceR() { return bLedgeHasFootSurfaceR; }
	FORCEINLINE void SetLedgeHasFootSurfaceR(bool Value) { bLedgeHasFootSurfaceR = Value; }

	FORCEINLINE bool GetLedgeHasFootSurfaceL() { return bLedgeHasFootSurfaceL; }
	FORCEINLINE void SetLedgeHasFootSurfaceL(bool Value) { bLedgeHasFootSurfaceL = Value; }


	/* HandIK */
	FORCEINLINE bool GetOverrideHandIK() { return bOverrideHandIK; }
	FORCEINLINE void SetOverrideHandIK(bool Value) { bOverrideHandIK = Value; }

	FORCEINLINE bool GetLedgeHasHandSurfaceL() { return bLedgeHasHandSurfaceL; }
	FORCEINLINE void SetLedgeHasHandSurfaceL(bool Value) { bLedgeHasHandSurfaceL = Value; }

	FORCEINLINE bool GetLedgeHasHandSurfaceR() { return bLedgeHasHandSurfaceR; }
	FORCEINLINE void SetLedgeHasHandSurfaceR(bool Value) { bLedgeHasHandSurfaceR = Value; }

	FORCEINLINE FVector GetHandIKLocationR() { return HandIKLocationR; }
	FORCEINLINE void SetHandIKLocationR(FVector NewLocation) { HandIKLocationR = NewLocation; }

	FORCEINLINE FVector GetHandIKLocationL() { return HandIKLocationL; }
	FORCEINLINE void SetHandIKLocationL(FVector NewLocation) { HandIKLocationL = NewLocation; }



public:

	/*
	UFUNCTION(BlueprintCallable)
	bool TryNextParkourAction(EInputType InputType);

	UFUNCTION(BlueprintCallable)
	UParkourAction* FindNextAction(EInputType InputType);
	*/
	UFUNCTION(BlueprintCallable)
	void OnStartParkourAction(UParkourAction* NewAction);

	UFUNCTION(BlueprintCallable)
	void OnEndParkourAction();



protected:
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Setting|Actions")
	TArray<UParkourAction*> ActionList;

	TMap<EInputType, TArray<UParkourAction*>> InputActionMap;

	UParkourAction* CurrentAction;

};
