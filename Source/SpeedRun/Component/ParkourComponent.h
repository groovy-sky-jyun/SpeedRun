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
	PARKOUR_None,
	PARKOUR_Hurdle,
	PARKOUR_Vault,
	PARKOUR_Mantle,
	PARKOUR_Hang,
	PARKOUR_WallRun
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
struct FObstacleData : public FTableRowBase
{
	GENERATED_BODY()

	// FrontLedge Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasFrontLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector FrontLedgeLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector FrontLedgeNormal;

	// UpperSurface Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasUpperSurface = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector UpperSurfaceLocation;

	// BackLedge Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasBackLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector BackLedgeLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector BackLedgeNormal;

	// LandingSurface Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasLandingSurface = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LandingSurfaceLocation;

	// Obstacle Value
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrontHeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BackDropHeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Depth = 0.f;

	
};

USTRUCT(BlueprintType)
struct FStepBoxData : public FTableRowBase
{
	GENERATED_BODY()

	// Next Ledge Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasNextFrontLedge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector NextFrontLedgeLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector NextFrontLedgeNormal;

	// Landing Surface Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasLandingSurface = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LandingSurfaceLocation;

	// Step Box Value
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GapDepth = 0.f;
};

USTRUCT(BlueprintType)
struct FEnvData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FObstacleData Obstacle_Data;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FStepBoxData StepBox_Data;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UPrimitiveComponent> HitComponent;
};

USTRUCT(BlueprintType)
struct FTraversalChooserParams : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ObstacleHeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ObstacleDepth = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GapDepth = 0.f;

public:
	void UpdateTraversalChooserParams(const FEnvData& CheckResult, ACharacter* Player);
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
	bool TryUpdateEnvData(FEnvData& EnvData);

	UFUNCTION(BlueprintCallable, Category = "Action")
	EParkourActionType EvaluateNextAction(const FEnvData& InEnvData);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void ExecuteMontageByActionType(const EParkourActionType ActionType, const FEnvData& InEnvData);

	UFUNCTION(BlueprintCallable, Category = "Action")
	UAnimMontage* SelectActionMontageFromCHT(UChooserTable* CHT, const FEnvData& InEnvData);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void SetupMotionWarping(const EParkourActionType ActionType, const FEnvData& InEnvData);

	void AddWarpTarget(FName TargetName, FVector Location, FVector Normal);


public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|Anim")
	TMap<EParkourActionType, UChooserTable*> ActionToCHT;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|Value")
	float MaxStepBoxGapDistance = 650.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|Value")
	float MaxObstacleDepth = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|Value")
	float MaxObstacleHeight_Vault = 250;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|Value")
	float MaxObstacleHeight_Hang = 500;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|Value")
	float MaxObstacleHeight_Mantle = 400;

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
	EParkourActionType CurrentAction = EParkourActionType::PARKOUR_None;

	UPROPERTY(VisibleAnywhere, Category = "Data")
	bool bCanParkour = false;

	float CapsuleRadius;
	float CapsuleHalfHeight;

private:
	//===== 장애물 및 환경 감지 =====//
	FHitResult TryDetectObstacle(FVector ActorLocation, FVector ActorForward);
	FHitResult TryDetectStepBox(FVector ActorLocation, FVector ActorForward);
	bool UpdateObstacleData(FHitResult ObstacleHitResult, AParkourBlock* Block, FEnvData& EnvData, FVector ActorLocation);
	bool UpdateStepBoxData(FVector EdgeLocation, float Radius, FEnvData& EnvData, FVector ActorForward);
	FHitResult ScanSurfaceEdge(ETraceDirection TraceDir, int32 Count, FVector Start, FVector Dir, float Distance, float GapSize, float Radius, bool bReturnHit, bool bDrawDebug) const;
	float GetDistance(const FVector& StartLocation, const FVector& EndLocation, const FVector& Normal);

	//===== Basic Trace Logic =====//
	FHitResult SphereTrace(const FVector& Start, const FVector& End, float Radius, bool bDrawDebug) const;
	FHitResult BoxTrace(const FVector& Start, const FVector& End, FVector BoxHalfSize, FRotator Rotation, bool bDrawDebug) const;
	FHitResult LineTrace(const FVector& Start, const FVector& End, bool bDrawDebug) const;
	FHitResult CapsuleTrace(const FVector& Start, const FVector& End, float Radius, float HalfHeight, bool bDrawDebug) const;
	void DrawSphereTrace(const FVector& Center, float Radius, float LifeTime) const;


};
