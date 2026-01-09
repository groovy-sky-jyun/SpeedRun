// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourHangShimmy.h"
#include "ParkourManager.h"

void UParkourHangShimmy::Initialize(ASpeedRunCharacter* OwnerPlayer, UParkourManager* ParkourComponent)
{
	Super::Initialize(OwnerPlayer, ParkourComponent);
}

bool UParkourHangShimmy::CheckVisibleToAction()
{
	if (ParkourManager->GetIsOnLedge())
	{
		return true;
	}

	return false;
}

void UParkourHangShimmy::OnStart()
{
	/*
	if (MovementVector.Y > 0.f)
	{
		DoHangUp();
	}
	else if (MovementVector.X != 0.f)
	{
		DoShimmy(MovementVector.X);
	}*/
}

void UParkourHangShimmy::OnUpdate()
{
}

void UParkourHangShimmy::OnEnd()
{
}
