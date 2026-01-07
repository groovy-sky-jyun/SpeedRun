// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourDrop.h"
#include "ParkourManager.h"
#include "GameFramework/CharacterMovementComponent.h"


void UParkourDrop::Initialize(ASpeedRunCharacter* OwnerPlayer, UParkourManager* ParkourComponent)
{
	Super::Initialize(OwnerPlayer, ParkourComponent);
}

bool UParkourDrop::CheckVisibleToAction()
{
	if (ParkourManager->GetIsOnLedge())
	{
		return true;
	}

	return false;
}

void UParkourDrop::OnStart()
{
	ParkourManager->SetIsOnLedge(false);
	ParkourManager->SetOverrideFootIK(false);
	ParkourManager->SetOverrideHandIK(false);

	Movement->SetMovementMode(EMovementMode::MOVE_Falling);
}

void UParkourDrop::OnUpdate()
{
}

void UParkourDrop::OnEnd()
{
}
