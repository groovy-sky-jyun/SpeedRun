// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "EnhancedInputComponent.h"
#include "ParkourComponent.generated.h"

class ASpeedRunCharacter;
class UCharacterMovementComponent;
class UCapsuleComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVaultMotionWarping);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPEEDRUN_API UParkourComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UParkourComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;



protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Owner")
	TObjectPtr<ASpeedRunCharacter> Player;

	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> PlayerMovement;

	FGameplayTagContainer* TagContainer;



public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Initialize input action bindings */
	void SetupParkourInputComponent(class UEnhancedInputComponent* ParkourInputComponent);



protected:

	/* Jump Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* UpAction;

	/* [Crouch/Slide/Drop/Roll] */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* DownAction;

	/* Dash Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SprintAction;

	/* [Interaction] */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InteractAction;



public: /* Called when the Player State Changes for input key */

	UFUNCTION(BlueprintCallable, Category = "Input")
	void Input_Up_Start(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void Input_Up_End(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void Input_Down(const FInputActionValue& Value);

	/** Called for dashing input */
	void Sprint(const FInputActionValue& Value);

	/* Interation Key Actions */
	UFUNCTION()
	void Interaction();



protected:
	/* Up Key Actions */
	UFUNCTION()
	void ShimmyJump();

	UFUNCTION()
	void RopeJump();

	UFUNCTION()
	void Mantle();

	UFUNCTION()
	void WallJump();

	UFUNCTION()
	bool TryParkourJump();
	/*---*/


	/* Down Key Actions */
	UFUNCTION()
	void StartSliding();

	UFUNCTION()
	void EndSliding();

	UFUNCTION()
	void Crouch();

	UFUNCTION()
	void UnCrouch();

	UFUNCTION()
	void Drop();

	UFUNCTION()
	void Roll();
	/*---*/


	/* Sprint Key Actions */
	UFUNCTION()
	void StartDash();

	UFUNCTION()
	void EndDash();

	UFUNCTION()
	void StartAirDash();

	UFUNCTION()
	void EndAirDash();
	/*---*/



	// Move Function
	UFUNCTION()
	void DoJump();

	UFUNCTION()
	void DoMiddleJump(const FVector& HitLocation);

	UFUNCTION()
	void DoHighJump(const FHitResult& HitResult);

	UFUNCTION()
	void DoVault();

	UFUNCTION()
	void DoMantle();

	UFUNCTION()
	void DoHang();

	UFUNCTION()
	bool CanSliding();
	//----------------



protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
	float DashDistance = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault|Trace|Count")
	int32 TraceCount_BlockHeight = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault|Trace|Count")
	int32 TraceCount_BlockVertical = 4;

	// Trace 사이의 간격
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault|Trace|Gap")
	float TraceGap_BlockHeight = 30.f;

	// Trace 사이의 간격
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault|Trace|Gap")
	float TraceGap_BlockVertical = 50.f;

	// 후에 anim 동작이랑 길이 맞추기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault|Trace|Length")
	float TraceLength_BlockHeight = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vault|Trace|Length")
	float TraceLength_BlockVertical = 50.f;



public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionWarping|Pos")
	FVector VaultStartPos;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionWarping|Pos")
	FVector VaultMiddlePos;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionWarping|Pos")
	FVector VaultLandPos;



/* Motion Warping State */
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionWarping|State")
	bool bCanWarp = false;


public:
	UPROPERTY(BlueprintAssignable, Category="MotionWarping")
	FOnVaultMotionWarping OnVaultMotionWarping;


/* Tags|State */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags|State", Meta = (Categories = "State.Parkour"))
	FGameplayTag WallRunningTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags|State", Meta = (Categories = "State.Parkour"))
	FGameplayTag HangTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags|State", Meta = (Categories = "State.Parkour"))
	FGameplayTag RopeTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags|State", Meta = (Categories = "State.Parkour"))
	FGameplayTag ShimmyTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags|State", Meta = (Categories = "State.Movement"))
	FGameplayTag FallingTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags|State", Meta = (Categories = "State.Status"))
	FGameplayTag CrouchedTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags|State", Meta = (Categories = "State.Status"))
	FGameplayTag SlidingTag;

	/* Tags|State */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags|Action", Meta = (Categories = "Action.Movement"))
	FGameplayTag DashTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags|Action", Meta = (Categories = "Action.Movement"))
	FGameplayTag AirDashTag;




/// <summary>
/// ////////
/// </summary>
protected:
	UFUNCTION()
	bool CheckHitWall();

	UFUNCTION()
	bool IsWallJump();

	/* 양쪽 벽 번갈아 타고 올라가는 동작 */
	UFUNCTION()
	void DoWallJump();

	UFUNCTION()
	void SlowJumpToLand();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Wall")
	bool bIsWallRun;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vault")
	bool bIsVaultJump;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vault|Animation")
	FVector FirstVaultingPos;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vault|Animation")
	FVector MiddleVaultingPos;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Vault|Animation")
	FVector LastVaultingPos;
};
