// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ParkourActionBase.h"
#include "ParkourAction_Mantle.generated.h"

class ASpeedRunCharacter;

UCLASS()
class SPEEDRUN_API UParkourAction_Mantle : public UParkourActionBase
{
	GENERATED_BODY()

public:
	virtual float Evaluate(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const override;
	virtual void ExecuteAction(UParkourComponent* Component, const FEnvData& EnvData, ASpeedRunCharacter* Player) const override;

public:
	// 낮은 장애물 / 높은 장애물을 가르는 경계.
	// 이 값 이하이면서 두꺼우면 "낮지만 못 넘는 장애물", 초과하면 "올라서는 장애물" 로 본다.
	// ParkourAction_Vault 의 MaxHeight 와 정확히 같을 필요는 없다 (어긋나도 아래 분기가 받아낸다).
	UPROPERTY(EditAnywhere, Category = "Mantle")
	float LowObstacleMaxHeight = 150.f;

	// Vault 가 두께 때문에 거부한 장애물을 받아내는 기준.
	// ParkourAction_Vault 의 MaxDepth 보다 크게 하면 그 사이 두께가 어느 액션에도 안 걸린다.
	// 작게 두는 건 안전하다 (Vault 와 겹치고 RegisteredActions 배열 순서로 결정됨).
	UPROPERTY(EditAnywhere, Category = "Mantle")
	float VaultMaxDepth = 200.f;

	// 올라설 수 있는 최대 높이
	UPROPERTY(EditAnywhere, Category = "Mantle")
	float MaxHeight = 250.f;

	// 올라설 공간 최소 두께
	UPROPERTY(EditAnywhere, Category = "Mantle")
	float MinDepth = 2.0f;

};
