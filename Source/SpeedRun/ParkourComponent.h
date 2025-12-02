// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ParkourComponent.generated.h"

class ASpeedRunCharacter;
class UCharacterMovementComponent;
class UCapsuleComponent;

UENUM(BlueprintType)
enum class EState : uint8 {
	None,
	WallRun,
	Vault,
	Mantle,
	Slide,
	Hang
};

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

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Owner")
	TObjectPtr<ASpeedRunCharacter> Player;

	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> PlayerMovement;


public: /* Called when the Player State Changes for input key */

	UFUNCTION()
	void DashOrSlide();

	UFUNCTION()
	void CrouchOrDrop();

	UFUNCTION()
	void ParkourJump();
	

protected:
	UFUNCTION()
	void StartDash();

	UFUNCTION()
	void EndDash();

	UFUNCTION()
	void StartSliding();

	UFUNCTION()
	void EndSliding();

	UFUNCTION()
	void DoCrouch();

	UFUNCTION()
	void DoDrop();

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


protected:
	UFUNCTION()
	bool CanSliding();

	UFUNCTION()
	bool CanJumping();

	UFUNCTION()
	bool CanVaulting();


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	bool bIsDashing;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	bool bIsSliding;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	bool bIsCrouch;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	bool bIsDrop;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	bool bIsVaulting;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	bool bIsMantling;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	bool bIsHanging;


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
