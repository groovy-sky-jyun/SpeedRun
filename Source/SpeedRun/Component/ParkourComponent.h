// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Templates/Function.h"
#include "Engine/DataTable.h"
#include "DA_JumpAction.h"
#include "Chooser.h"
#include "ParkourComponent.generated.h"

class ASpeedRunCharacter;
class UParkourMovementComponent;
class UMotionWarpingComponent;
class UDA_AnimOption;
struct FAnimInfo;
class UDA_EnvironmentTags;
class UDA_ParkourActionCategory;
class UDA_TraceOptions;
class UDA_SphereTracesOption;
class UDA_BoxTraceOption;
class UDA_LineTraceOption;


UENUM(BlueprintType)
enum class ETraceDirection : uint8
{
	Horizontal,
	Vertical
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
struct FTraversalCheckResult : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FObstacleCheckResult Obstacle_Data;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FStepBoxCheckResult StepBox_Data;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UPrimitiveComponent> HitComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimMontage> ChosenMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayRate;
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
	void InitializeFromContext(const FTraversalCheckResult& CheckResult, ACharacter* Player);
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
	bool TryTraversalJumpAction();

	UFUNCTION(BlueprintCallable, Category = "Action")
	UAnimMontage* TryParkourChooser(FTraversalCheckResult& CheckResult);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void DoLanding();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void PlayAminMontage(const FTraversalCheckResult& TraversalResult) const;



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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChooserTable")
	UChooserTable* CHT_TraversalAnims;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Value")
	float MaxStepBoxGapDistance = 650.f;

private:
	//===== 장애물 및 환경 감지 =====//
	FHitResult TryDetectObstacle();
	FHitResult ScanSurfaceEdge(ETraceDirection TraceDir, int32 Count, FVector Start, FVector Dir, float Distance, float GapSize, float Radius, bool bReturnHit, bool bDrawDebug) const;
	bool IsOnStepBox();

	//===== Basic Trace Logic =====//
	FHitResult SphereTrace(const FVector& Start, const FVector& End, float Radius, bool bDrawDebug) const;
	FHitResult BoxTrace(FVector Start, FVector End, FVector BoxHalfSize, FRotator Rotation, bool bDrawDebug) const;
	FHitResult LineTrace(FVector Start, FVector End, bool bDrawDebug) const;
	FHitResult CapsuleTrace(FVector& Start, FVector& End, float Radius, float HalfHeight, bool bDrawDebug) const;
	void DrawSphereTrace(FVector Center, float Radius, float LifeTime) const;


};
