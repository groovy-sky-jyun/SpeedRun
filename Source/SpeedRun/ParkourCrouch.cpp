// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourCrouch.h"
#include "SpeedRunCharacter.h"
#include "ParkourManager.h"
#include "GameFramework/CharacterMovementComponent.h"

void UParkourCrouch::Initialize(ASpeedRunCharacter* OwnerPlayer, UParkourManager* ParkourComponent)
{
	Super::Initialize(OwnerPlayer, ParkourComponent);
}

bool UParkourCrouch::CheckVisibleToAction()
{
	if (ParkourManager->GetIsOnLedge() || Movement->IsFalling())
	{
		return false;
	}

	return true;
}

void UParkourCrouch::OnStart()
{
	Movement->bWantsToCrouch = ~(Movement->bWantsToCrouch);

	ParkourManager->SetIsCrouch(Movement->bWantsToCrouch);
}

void UParkourCrouch::OnUpdate()
{
}

void UParkourCrouch::OnEnd()
{
}
