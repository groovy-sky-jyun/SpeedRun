// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpeedRunCharacter.h"
#include "EnhancedInputComponent.h"
#include "ParkourDataAsset.h"
#include "ParkourActionComponent.generated.h"

class ASpeedRunPlayerController;
class UInputAction;
struct FInputActionValue;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPEEDRUN_API UParkourActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UParkourActionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;



protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Owner")
	TObjectPtr<ASpeedRunCharacter> Player;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Owner")
	TObjectPtr<ASpeedRunPlayerController> PlayerController;


public:
	UPROPERTY(EditAnywhere, Category = "Setting|Input")
	UInputAction* LedgeHangUpAction;

	UPROPERTY(EditAnywhere, Category = "Setting|Input")
	UInputAction* LedgeShimmyAction;

	UPROPERTY(EditAnywhere, Category = "Setting|Input")
	UInputAction* LedgeDropAction;

	UPROPERTY(EditAnywhere, Category = "Setting|Input")
	UInputAction* ParkourJumpAction;


public:
	UFUNCTION()
	void SetupParkourInputComponent(class UEnhancedInputComponent* EnhancedInputComponent);

	UFUNCTION(BlueprintCallable, Category = "InputActions")
	void HandleLedgeHangUp(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "InputActions")
	void HandleLedgeShimmy(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "InputActions")
	void HandleLedgeDrop(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "InputActions")
	void HandleParkourJump(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "InputActions")
	bool CanParkourJump(const FInputActionValue& Value);

	UFUNCTION()
	void DetectAnything();



public:
	UFUNCTION()
	FVector MoveVectorUpward(FVector Vector, float ZOffset);

	UFUNCTION()
	FVector MoveVectorDownward(FVector Vector, float ZOffset);

	UFUNCTION()
	FVector MoveVectorForward(FVector Vector, FRotator Rotation, float Distance);

	UFUNCTION()
	FVector MoveVectorBackward(FVector Vector, FRotator Rotation, float Distance);

	UFUNCTION()
	FVector MoveVectorLeft(FVector Vector, FRotator Rotation, float Distance);

	UFUNCTION()
	FVector MoveVectorRight(FVector Vector, FRotator Rotation, float Distance);

	UFUNCTION()
	FRotator ReverseNormal(FVector Normal);



public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|DA")
	TArray<UParkourDataAsset*> ActionLibrary;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setting|DA")
	FGameplayTagContainer CurrentActionTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setting|DA")
	FGameplayTagContainer ConditionTags;

	UFUNCTION(BlueprintCallable, Category="Parkour")
	void TryParkourAction();


protected:
	FORCEINLINE void AddActionTag(FGameplayTag NewTag) { CurrentActionTag.AddTag(NewTag); }
	FORCEINLINE void RemoveActionTag(FGameplayTag Tag) { CurrentActionTag.RemoveTag(Tag); }
	FORCEINLINE void ClearActionTags() { CurrentActionTag.Reset(); }


	FORCEINLINE void AddConditionTag(FGameplayTag NewTag) { ConditionTags.AddTag(NewTag); }
	FORCEINLINE void RemoveConditionTag(FGameplayTag Tag) { ConditionTags.RemoveTag(Tag); }
	FORCEINLINE void ClearConditionTags() { ConditionTags.Reset(); }

	UFUNCTION()
	UParkourDataAsset* GetBestActionMatch();

};
