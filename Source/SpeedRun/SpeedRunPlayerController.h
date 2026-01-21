// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SpeedRunPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class ASpeedRunPlayerController : public APlayerController
{
	GENERATED_BODY()
	

protected:
	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category = "InputMappingContext")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	UPROPERTY(EditAnywhere, Category = "InputMappingContext")
	TArray<UInputMappingContext*> ParkourMappingContexts;



public:
	UFUNCTION(BlueprintCallable, Category = "InputMappingContext")
	void UpdateParkourMappingContext(bool Value);

	UFUNCTION(BlueprintCallable, Category = "InputMappingContext")
	bool HasParkourIMC();


private:
	UPROPERTY()
	bool bHasParkourIMC;
};
