// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourAction.h"
#include "SpeedRunCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "ParkourManager.h"


void UParkourAction::Initialize(ASpeedRunCharacter* OwnerPlayer, UParkourManager* ParkourComponent)
{
    Player = OwnerPlayer;
    ParkourManager = ParkourComponent;

    if (Player)
    {
		Movement = Player->GetCharacterMovement();
		Capsule = Player->GetCapsuleComponent();
        WarpComponent = Player->GetMotionWarpingComponent();

		if (Capsule)
		{
			CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}
    }
}

const UWorld* UParkourAction::GetPlayerWorld()
{
    if (GetOuter())
    {
        return GetOuter()->GetWorld();
    }

    return nullptr;
}

