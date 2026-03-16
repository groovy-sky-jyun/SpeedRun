// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Templates/Function.h"
#include "Engine/DataTable.h"
#include "Chooser.h"
#include "ParkourComponent.generated.h"

class ASpeedRunCharacter;
class UParkourMovementComponent;
class UMotionWarpingComponent;
class AParkourBlock;


UENUM(BlueprintType)
enum class EParkourActionType : uint8
{
	None,
	Hurdle,
	Vault,
	Hang,
	WallRun
};

UENUM(BlueprintType)
enum class ETraceDirection : uint8
{
	Horizontal,
	Vertical
};

USTRUCT(BlueprintType)
struct FParkourActionData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EParkourActionType ActionType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimMontage> AnimMontage;
};

USTRUCT(BlueprintType)
struct FObstacleCheckResult : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasFrontLedge;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector FrontLedgeLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector FrontLedgeNormal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasBackLedge;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector BackLedgeLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector BackLedgeNormal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BackLedgeHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasBackFloor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector BackFloorLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ObstacleHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ObstacleDepth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasUpperSurface;

};

USTRUCT(BlueprintType)
struct FStepBoxCheckResult : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOnEdge; //캐릭터 발밑 아래로 홈이 파여있는지 확인

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasNextFrontLedge;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector NextFrontLedgeLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector NextFrontLedgeNormal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GapDepth;
};

USTRUCT(BlueprintType)
struct FEnvironmentData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FObstacleCheckResult Obstacle_Data;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FStepBoxCheckResult StepBox_Data;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UPrimitiveComponent> HitComponent;
};

USTRUCT(BlueprintType)
struct FTraversalChooserParams : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EMovementMode> MovementMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasFrontLedge;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasUpperSurface;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasBackFloor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOnEdge;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasLandingSurface;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ObstacleHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ObstacleDepth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GapDepth;

public:
	void UpdateTraversalChooserParams(const FEnvironmentData& CheckResult, ACharacter* Player);
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
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//===== 액션 수행 (Execution) =====//
	UFUNCTION(BlueprintCallable, Category = "Action")
	void PerformJumpSequence();

	UFUNCTION(BlueprintCallable, Category = "Action")
	bool TryUpdateEnvData();

	UFUNCTION(BlueprintCallable, Category = "Action")
	EParkourActionType EvaluateNextAction(const FEnvironmentData& InCurrentEnvData);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void ExecuteMontageByActionType(const EParkourActionType ActionType, const FEnvironmentData& InCurrentEnvData);

	UFUNCTION(BlueprintCallable, Category = "Action")
	UAnimMontage* SelectActionMontageFromCHT(UChooserTable* CHT, const FEnvironmentData& InCurrentEnvData);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void SetupMotionWarping() const;


public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|Anim")
	TMap<EParkourActionType, UChooserTable*> ActionToCHT;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|Value")
	float MaxStepBoxGapDistance = 650.f;


protected:
	//===== 참조 및 상태 변수 =====//
	UPROPERTY()
	TObjectPtr<ASpeedRunCharacter> Player;

	UPROPERTY()
	TObjectPtr<class UParkourMovementComponent> ParkourMovement;

	UPROPERTY()
	TObjectPtr<UAnimInstance> AnimInstance;

	UPROPERTY()
	TObjectPtr<UMotionWarpingComponent> WarpComponent;


private:
	UPROPERTY(VisibleAnywhere, Category = "Data")
	EParkourActionType CurrentParkourAction = EParkourActionType::None;

	UPROPERTY(VisibleAnywhere, Category = "Data")
	FEnvironmentData CurrentEnvData = {};

	UPROPERTY(VisibleAnywhere, Category = "Data")
	bool bCanParkour = false;

private:
	//===== 장애물 및 환경 감지 =====//
	FHitResult TryDetectObstacle(FVector ActorLocation, FVector ActorForward, float CapsuleHalfHeight);
	bool UpdateObstacleData(FHitResult ObstacleHitResult, AParkourBlock* Block, FEnvironmentData& TraversalResult, FVector ActorLocation, float CapsuleRadius, float CapsuleHalfHeight);
	FHitResult TryDetectStepBox(FVector ActorLocation, FVector ActorForward, float CapsuleHalfHeight);
	bool UpdateStepBoxData(FVector EdgeLocation, float Radius, FEnvironmentData& TraversalResult, FVector ActorForward);
	FHitResult ScanSurfaceEdge(ETraceDirection TraceDir, int32 Count, FVector Start, FVector Dir, float Distance, float GapSize, float Radius, bool bReturnHit, bool bDrawDebug) const;


	//===== Basic Trace Logic =====//
	FHitResult SphereTrace(const FVector& Start, const FVector& End, float Radius, bool bDrawDebug) const;
	FHitResult BoxTrace(FVector Start, FVector End, FVector BoxHalfSize, FRotator Rotation, bool bDrawDebug) const;
	FHitResult LineTrace(FVector Start, FVector End, bool bDrawDebug) const;
	FHitResult CapsuleTrace(FVector& Start, FVector& End, float Radius, float HalfHeight, bool bDrawDebug) const;
	void DrawSphereTrace(FVector Center, float Radius, float LifeTime) const;


};
