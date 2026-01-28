// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Engine/EngineTypes.h"
#include "AN_SetMovementMode.generated.h"

/**
 * 
 */
UCLASS()
class SPEEDRUN_API UAN_SetMovementMode : public UAnimNotifyState
{
	GENERATED_BODY()
	

public:
    UPROPERTY(EditAnywhere, Category = "Settings")
    TEnumAsByte<EMovementMode> BeginMode = MOVE_None;

    UPROPERTY(EditAnywhere, Category = "Settings")
    TEnumAsByte<EMovementMode> EndMode = MOVE_None;

    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
