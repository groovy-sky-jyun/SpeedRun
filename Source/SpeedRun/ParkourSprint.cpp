// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourSprint.h"
#include "SpeedRunCharacter.h"
#include "ParkourManager.h"
#include "GameFramework/CharacterMovementComponent.h"

void UParkourSprint::Initialize(ASpeedRunCharacter* OwnerPlayer, UParkourManager* ParkourComponent)
{
	Super::Initialize(OwnerPlayer, ParkourComponent);
}

bool UParkourSprint::CheckVisibleToAction()
{
	if (ParkourManager->GetIsOnLedge() || Movement->IsFalling())
	{
		return false;
	}

	
	return true;
}

void UParkourSprint::OnStart()
{
	FVector ForwardDir = Player->GetActorForwardVector();
	Player->LaunchCharacter(ForwardDir * DashDistance, true, true);
	
	/* play the dash montage 
	if (UAnimInstance* AnimInstance = Player->GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Play(SprintMontage);
	}*/
}

void UParkourSprint::OnUpdate()
{
}

void UParkourSprint::OnEnd()
{
}
