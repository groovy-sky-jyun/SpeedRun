// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourShimmy.h"
#include "ParkourManager.h"
#include "ParkourMovementComponent.h"

void UParkourShimmy::Initialize(ASpeedRunCharacter* OwnerPlayer, UParkourManager* ParkourComponent)
{
	Super::Initialize(OwnerPlayer, ParkourComponent);

	bIsShimmy = false;
}

bool UParkourShimmy::CheckVisibleToAction(FVector WorldDirection, float ScaleValue)
{
	if (ParkourManager->GetIsOnLedge() && !IsShimmy())
	{
		float InitialZOffset = InitialZOffset_Falling;
		float TraceVertical = TraceVertical_Falling;

		return CheckDetectToLedge(InitialZOffset, TraceDistanceH, TraceVertical, ScaleValue);
	}
	return false;
}

void UParkourShimmy::OnStart()
{
	bIsShimmy = true;

	HangOnLedge();
}

void UParkourShimmy::OnUpdate()
{
}

void UParkourShimmy::OnEnd()
{
}

bool UParkourShimmy::IsShimmy()
{
	return bIsShimmy;
}
