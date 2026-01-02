// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ParkourAction.h"
#include "ParkourHang.generated.h"

class UAnimMontage;

UCLASS()
class SPEEDRUN_API UParkourHang : public UParkourAction
{
	GENERATED_BODY()
	

public:
	virtual bool CheckVisibleToAction() override;

	virtual void OnStart() override;

	virtual void OnUpdate() override;

	virtual void OnEnd() override;



private:
	UFUNCTION()
	bool CheckDetectToLedge(float InitialZOffset, float TraceDistance, float TraceVertical);

	UFUNCTION()
	void HangOnLedge();

	UFUNCTION()
	void CalculateTargetRotatorAndLocation(FRotator& TargetRotator, FVector& TargetLocation);

	UFUNCTION()
	void CheckIfBelowLedgeHasSurface();

	UFUNCTION()
	void LedgeJump(FRotator& TargetRotator, FVector& TargetLocation);


private:
	UPROPERTY()
	FVector DetectLedgeLocation;

	UPROPERTY()
	FVector DetectLedgeNormal;

	FTimerHandle CheckLedfeSurfaceHandle;

	FTimerHandle LedgeIKHandle;



protected:
	UPROPERTY(EditAnywhere, Category = "Trace")
	float InitialZOffset_Grounded = 75.f;

	UPROPERTY(EditAnywhere, Category = "Trace")
	float TraceVertical_Grounded = 15.f;

	UPROPERTY(EditAnywhere, Category = "Trace")
	float InitialZOffset_Falling = 50.f;

	UPROPERTY(EditAnywhere, Category = "Trace")
	float TraceVertical_Falling = 20.f;

	float TraceDistanceH = 100.f;



protected:
	UPROPERTY(EditDefaultsOnly, Category = "AnimMontage")
	TObjectPtr<UAnimMontage> IdleToBracedHang;

	UPROPERTY(EditDefaultsOnly, Category = "AnimMontage")
	TObjectPtr<UAnimMontage> IdleToFreeHang;


};
