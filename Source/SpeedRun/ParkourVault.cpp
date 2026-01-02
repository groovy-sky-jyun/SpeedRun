// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourVault.h"
#include "SpeedRunCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "ParkourManager.h"
#include "Animation/AnimMontage.h"
#include "MotionWarpingComponent.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"

void UParkourVault::Initialize(ASpeedRunCharacter* OwnerPlayer, UParkourManager* ParkourComponent)
{
	UParkourAction::Super;
}

bool UParkourVault::CheckVisibleToAction()
{
	//float InitialZOffset = Movement->IsFalling() ? InitialZOffset_Falling : InitialZOffset_Grounded;
	//float TraceVertical = Movement->IsFalling() ? TraceVertical_Falling : TraceVertical_Grounded;
	//return CheckDetectToLedge(InitialZOffset, TraceDistanceH, TraceVertical);
	return false;
}

void UParkourVault::OnStart()
{
}

void UParkourVault::OnUpdate()
{
}

void UParkourVault::OnEnd()
{
}
