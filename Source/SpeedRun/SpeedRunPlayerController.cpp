// Copyright Epic Games, Inc. All Rights Reserved.


#include "SpeedRunPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "SpeedRun.h"
#include "Widgets/Input/SVirtualJoystick.h"

void ASpeedRunPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ASpeedRunPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
		}
	}
}

void ASpeedRunPlayerController::UpdateParkourMappingContext(bool Value)
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (Value)
		{
			Subsystem->AddMappingContext(ParkourMappingContext, 1);
		}
		else
		{
			Subsystem->RemoveMappingContext(ParkourMappingContext);
		}
	}
	
}

