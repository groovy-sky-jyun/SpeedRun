// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState_ParkourPhysics.h"
#include "SpeedRunCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

void UAnimNotifyState_ParkourPhysics::NotifyBegin(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComponent, Animation, TotalDuration, EventReference);

	if (MeshComponent && MeshComponent->GetOwner())
	{
		if (ASpeedRunCharacter* Character = Cast<ASpeedRunCharacter>(MeshComponent->GetOwner()))
		{
			if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
			{
				// 1.기존에 남아있던 가속도나 관성 완벽하게 제거
				MovementComponent->StopMovementImmediately();
				
				MovementComponent->SetMovementMode(EMovementMode::MOVE_Flying);
				Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
			}
		}
	}
}

void UAnimNotifyState_ParkourPhysics::NotifyEnd(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComponent, Animation, EventReference);

	if (MeshComponent && MeshComponent->GetOwner())
	{
		if (ASpeedRunCharacter* Character = Cast<ASpeedRunCharacter>(MeshComponent->GetOwner()))
		{
			if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
			{
				// 걷기(중력) 상태로 복구
				MovementComponent->SetMovementMode(EMovementMode::MOVE_Walking);
				Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
			}
		}
	}
}
