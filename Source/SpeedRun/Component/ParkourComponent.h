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

UENUM(BlueprintType)
enum class ETraceType : uint8
{
	Sphere,
	Box,
	Line
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasLandingSurface;
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
	void InitTraceMap();
	void InitTagMap();

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//===== 액션 수행 (Execution) =====//

	UFUNCTION(BlueprintCallable, Category = "Action")
	bool TryTraversalJumpAction(float TraceForwardDistance);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void PerformTraversalAction();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void UpdateWarpTarget();

	UFUNCTION(BlueprintCallable, Category="Action")
	void HandleToJump();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void DoLanding();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void PlayAminMontage(FGameplayTag TagCategory);



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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Structure")
	FTraversalCheckResult S_TraversalCheckResult;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ChooserTable")
	TObjectPtr<UChooserTable> CHT_TraversalAnims;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tag|State")
	FGameplayTagContainer CurrentActionTags;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tag|State")
	FGameplayTagContainer CurrentEnvironmentTags;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tag|Map")
	TMap<FGameplayTag, TObjectPtr<UDA_AnimOption>> AnimInfoMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tag|Map")
	TMap<FGameplayTag, TObjectPtr<UDA_ParkourActionCategory>> ActionCategoryMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tag|Map")
	TMap<FGameplayTag, FJumpOption> JumpConfigMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tag|Map")
	TMap<FGameplayTag, TObjectPtr<UDA_EnvironmentTags>> EnvironmentMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tag|Map")
	TMap<FGameplayTag, TObjectPtr<UDA_SphereTracesOption>> SphereTraceMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tag|Map")
	TMap<FGameplayTag, TObjectPtr<UDA_BoxTraceOption>> BoxTraceMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tag|Map")
	TMap<FGameplayTag, TObjectPtr<UDA_LineTraceOption>> LineTraceMap;



	//===== DataAsset =====//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DataAsset|Action")
	TArray<TObjectPtr<UDA_AnimOption>> DA_ActionAnimInfo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataAsset|Action")
	TArray<TObjectPtr<UDA_ParkourActionCategory>> DA_ActionCategoryList;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataAsset|Action")
	TObjectPtr<UDA_JumpAction> DA_JumpConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataAsset|Environment")
	TArray<TObjectPtr<UDA_EnvironmentTags>> DA_EnvironmentTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataAsset|Trace")
	TArray<TObjectPtr<UDA_TraceOptions>> TracesOptions;



	//===== GameplayTag =====//
	/**[Obstacle]**/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag|Obstacle")
	FGameplayTag Tag_Detect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag|Obstacle")
	FGameplayTag Tag_DetectNone;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag|Obstacle")
	FGameplayTag Tag_DetectObstacle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag|Obstacle")
	FGameplayTag Tag_ObstacleHeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag|Obstacle")
	FGameplayTag Tag_ObstacleWidth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag|Obstacle")
	FGameplayTag Tag_ObstacleLand;

	/**[Surface]**/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag|Surface")
	FGameplayTag Tag_StepBoxGap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag|Surface")
	FGameplayTag Tag_BuildingGap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag|Surface")
	FGameplayTag Tag_SurfaceSpace;

	/**[Action]**/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag|Action")
	FGameplayTag Tag_Jump;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag|Action")
	FGameplayTag Tag_Vault;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag|Action")
	FGameplayTag Tag_Landing;


public:
	//===== Character Physics State =====//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool bCanLanding = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool bCanVault = false;

	UFUNCTION(BlueprintCallable, Category = "Components")
	FORCEINLINE bool GetCanVault() const { return bCanVault; } 

	UFUNCTION(BlueprintCallable, Category = "Components")
	FORCEINLINE void SetCanVault(bool Value) { bCanVault = Value; }


private:
	//===== Find DataAsset (feat.Tag) =====//
	FGameplayTag SelectEnvTagOnContext(const FGameplayTag& TagCategory, float Value);
	FGameplayTag SelectActionTagOnContext(const FGameplayTag& ActionCategory);
	FAnimInfo FindAnimInfo(const FGameplayTag& TagCategory) const;



	//===== 장애물 및 환경 감지 =====//
	void ScanEnvironment();
	void ScanObstacleContext();
	void ScanEdgeContext();
	bool DetectObstacle();
	void ScanStepBoxContext(const FVector& StepOverStart);
	void ScanBuildingContext(const FVector& PlayerFootLocation);



	//===== Basic Trace Logic =====//
	FHitResult DetectSphereTraces(FGameplayTag TagCategory, FVector Start, FVector Dir, bool bReturnHit, ETraceDirection TraceDir, bool bDrawDebug) const;
	FHitResult SphereTrace(const FVector& Start, const FVector& End, float Radius, bool bDrawDebug) const;
	FHitResult BoxTrace(FGameplayTag TagCategory, FVector Start, FVector Dir, FRotator Rotation, bool bDrawDebug) const;
	FHitResult LineTrace(FGameplayTag TagCategory, ETraceDirection TraceDir, FVector Start, FVector Dir, bool bDrawDebug) const;
	


};
