// Fill out your copyright notice in the Description page of Project Settings.


#include "AN_SetMovementMode.h"
//#include "GameFramework/Character.h"
#include "SpeedRunCharacter.h"
#include "ParkourMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

void UAN_SetMovementMode::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
    if (ASpeedRunCharacter* Character = Cast<ASpeedRunCharacter>(MeshComp->GetOwner()))
    {
        Character->GetCharacterMovement()->SetMovementMode(BeginMode);
    }
}

void UAN_SetMovementMode::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);
    if (ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner()))
    {
        Character->GetCharacterMovement()->SetMovementMode(EndMode);
    }
}
