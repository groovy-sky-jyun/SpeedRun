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
	// Sets default values for this component's properties
	UParkourManager();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Initialize input action bindings */
	void SetupParkourInputComponent(class UEnhancedInputComponent* ParkourInputComponent);



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

	/* Jump Input Action */
	UPROPERTY(EditAnywhere, Category = "Setting|Input")
	UInputAction* UpAction;

	/* [Crouch/Slide/Drop/Roll] */
	UPROPERTY(EditAnywhere, Category = "Setting|Input")
	UInputAction* DownAction;

	/* Dash Input Action */
	UPROPERTY(EditAnywhere, Category = "Setting|Input")
	UInputAction* SprintAction;

	/* [Interaction] */
	UPROPERTY(EditAnywhere, Category = "Setting|Input")
	UInputAction* InteractAction;



public: /* Called when the Player State Changes for input key */

	UFUNCTION(BlueprintCallable, Category = "Input")
	void Input_Up_Start(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void Input_Up_End(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void Input_Down(const FInputActionValue& Value);

	/** Called for dashing input */
	void Sprint(const FInputActionValue& Value);

	/* Interation Key Actions */
	UFUNCTION()
	void Interaction();



public:
	UFUNCTION()
	UParkourAction* CheckPlayAction(EParkourStateType InputType);

	UFUNCTION()
	void PlayAction(UParkourAction* NewAction);

	UFUNCTION()
	void UpdateState(EParkourStateType InputType);





private:
	bool bCanMove = false;

	bool bIsOnLedge = false;

	bool bLedgeHasFootSurfaceL = false;

	bool bLedgeHasFootSurfaceR = false;

	bool bOverrideFootIK = false;



public:
	UFUNCTION()
	bool GetCanMove();

	UFUNCTION()
	void SetCanMove(bool Value);

	UFUNCTION()
	bool GetIsOnLedge();

	UFUNCTION()
	void SetIsOnLedge(bool Value);

	UFUNCTION()
	bool GetLedgeHasFootSurfaceR();

	UFUNCTION()
	void SetLedgeHasFootSurfaceR(bool Value);

	UFUNCTION()
	bool GetLedgeHasFootSurfaceL();

	UFUNCTION()
	void SetLedgeHasFootSurfaceL(bool Value);

	UFUNCTION()
	bool GetOverrideFootIK();

	UFUNCTION()
	void SetOverrideFootIK(bool Value);

};
