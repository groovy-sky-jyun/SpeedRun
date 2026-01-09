// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpeedRunCharacter.h"
#include "EnhancedInputComponent.h"
#include "ParkourAction.h"
#include "ParkourManager.generated.h"

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

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetupParkourInputComponent(class UInputComponent* ParkourInputComponent);

	UFUNCTION(BlueprintCallable)
	bool TryDetectParkour();

	UFUNCTION(BlueprintCallable)
	void SwitchToParkourInput(bool Value);


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Owner")
	TObjectPtr<ASpeedRunCharacter> Player;

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Setting|ParkourList")
	TArray<UParkourAction*> ActionList;



private:
	TMap<EParkourStateType, TArray<UParkourAction*>> InputActionMap; 

	UParkourAction* CurrentAction;

	EParkourStateType CurrentStateType;



protected:
	UPROPERTY(EditAnywhere, Category = "Setting|Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "Setting|Input")
	UInputAction* UpAction;

	UPROPERTY(EditAnywhere, Category = "Setting|Input")
	UInputAction* DownAction;

	UPROPERTY(EditAnywhere, Category = "Setting|Input")
	UInputAction* DashAction;


public: /* Called when the Player State Changes for input key */
	UFUNCTION(BlueprintCallable, Category = "InputActions")
	void HandleMove(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "InputActions")
	void HandleUp_Start(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "InputActions")
	void HandleUp_End(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "InputActions")
	void HandleDown(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "InputActions")
	void HandleDash(const FInputActionValue& Value);



public:
	UFUNCTION()
	UParkourAction* CheckPlayAction(EParkourStateType InputType);

	UFUNCTION()
	void PlayAction(UParkourAction* NewAction);

	UFUNCTION()
	void UpdateState(EParkourStateType InputType);





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
	FORCEINLINE bool GetCanMove() const { return bCanMove; }
	FORCEINLINE void SetCanMove(bool Value)  { bCanMove = Value; }

	FORCEINLINE bool GetIsCrouch() const { return bIsCrouch; }
	FORCEINLINE void SetIsCrouch(bool Value) { bIsCrouch = Value; }

	FORCEINLINE bool GetIsOnLedge() const { return bIsOnLedge; }
	FORCEINLINE void SetIsOnLedge(bool Value) { bIsOnLedge = Value; }


	/* FootIK */
	FORCEINLINE bool GetOverrideFootIK() const { return bOverrideFootIK; }
	FORCEINLINE void SetOverrideFootIK(bool Value) { bOverrideFootIK = Value; }

	FORCEINLINE bool GetLedgeHasFootSurfaceR() const { return bLedgeHasFootSurfaceR; }
	FORCEINLINE void SetLedgeHasFootSurfaceR(bool Value) { bLedgeHasFootSurfaceR = Value; }

	FORCEINLINE bool GetLedgeHasFootSurfaceL() const { return bLedgeHasFootSurfaceL; }
	FORCEINLINE void SetLedgeHasFootSurfaceL(bool Value) { bLedgeHasFootSurfaceL = Value; }


	/* HandIK */
	FORCEINLINE bool GetOverrideHandIK() const { return bOverrideHandIK; }
	FORCEINLINE void SetOverrideHandIK(bool Value) { bOverrideHandIK = Value; }

	FORCEINLINE bool GetLedgeHasHandSurfaceL() const { return bLedgeHasHandSurfaceL; }
	FORCEINLINE void SetLedgeHasHandSurfaceL(bool Value) { bLedgeHasHandSurfaceL = Value; }

	FORCEINLINE bool GetLedgeHasHandSurfaceR() const { return bLedgeHasHandSurfaceR; }
	FORCEINLINE void SetLedgeHasHandSurfaceR(bool Value) { bLedgeHasHandSurfaceR = Value; }

	FORCEINLINE FVector GetHandIKLocationR() const { return HandIKLocationR; }
	FORCEINLINE void SetHandIKLocationR(FVector NewLocation) { HandIKLocationR = NewLocation; }

	FORCEINLINE FVector GetHandIKLocationL() const { return HandIKLocationL; }
	FORCEINLINE void SetHandIKLocationL(FVector NewLocation) { HandIKLocationL = NewLocation; }
};
