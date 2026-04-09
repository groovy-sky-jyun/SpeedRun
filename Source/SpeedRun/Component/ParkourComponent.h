#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Templates/Function.h"
#include "Engine/DataTable.h"
#include "Chooser.h"
#include "GameplayTagContainer.h"
#include "ParkourComponent.generated.h"

class ASpeedRunCharacter;
class UParkourMovementComponent;
class UMotionWarpingComponent;
class AParkourBlock;


UENUM(BlueprintType)
enum class EParkourActionType : uint8
{
	PARKOUR_None,
	PARKOUR_Vault,
	PARKOUR_Mantle,
	PARKOUR_Hang,
	PARKOUR_WallRun,
	PARKOUR_Swing,
	PARKOUR_Pole,
	PARKOUR_WallSidle
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EParkourActionType ActionType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> AnimMontage;
};

USTRUCT(BlueprintType)
struct FObstacleData : public FTableRowBase
{
	GENERATED_BODY()

	// FrontLedge Data
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasFrontLedge = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector FrontLedgeLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector FrontLedgeNormal;

	// UpperSurface Data
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasUpperSurface = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector UpperSurfaceLocation;

	// BackLedge Data
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasBackLedge = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector BackLedgeLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector BackLedgeNormal;

	// LandingSurface Data
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasLandingSurface = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector LandingSurfaceLocation;

	// Obstacle Value
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float FrontHeight = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float BackDropHeight = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Depth = 0.f;

	
};

USTRUCT(BlueprintType)
struct FEnvData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag HitParkourTag = FGameplayTag::EmptyTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FObstacleData Obstacle_Data;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UPrimitiveComponent> HitComponent;
};

USTRUCT(BlueprintType)
struct FTraversalChooserParams : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Speed = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ObstacleHeight = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ObstacleDepth = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
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

	void AlignToLedge(const FEnvData& InEnvData);
	void AddWarpTarget(FName TargetName, FVector Location, FVector Normal);
	

	bool CanVault(const FEnvData& InEnvData, EMovementMode CurrentMode);
	bool CanMantle(const FEnvData& InEnvData, EMovementMode CurrentMode, uint8 CustomMode);
	bool CanHang(const FEnvData& InEnvData, EMovementMode CurrentMode);

	UFUNCTION(BlueprintCallable, Category = "Action|Hang")
	bool IsHanging() const { return bIsHanging; }

	UFUNCTION(BlueprintCallable, Category = "Action|Hang")
	void DropFromHang();


protected:
	void EnterHangState();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|Anim")
	TMap<EParkourActionType, UChooserTable*> ActionToCHT;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|Value")
	float MaxStepBoxGapDistance = 650.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|Value")
	float MaxObstacleHeight_Vault = 250;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|Value")
	float MaxObstacleHeight_Hang = 500;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|Value")
	float MaxObstacleHeight_Mantle = 400;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|Value")
	float MinHeightBlock = 50;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|Value")
	float MaxHeightVault = 150;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|Value")
	float MaxHeightMantle = 250;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setting|Value")
	float MaxDepthVault = 100;


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

	UPROPERTY(VisibleAnywhere, Category = "Data")
	bool bIsHanging = false;

	float CapsuleRadius;
	float CapsuleHalfHeight;

private:
	//===== 장애물 및 환경 감지 =====//
	FHitResult TryDetectObstacle(FVector ActorLocation, FVector ActorForward);
	bool UpdateObstacleData(FHitResult ObstacleHitResult, AParkourBlock* Block, FEnvData& EnvData, FVector ActorLocation);
	bool CanLanding(const FEnvData& InEnvData, FVector& LandingLocation, float& DropHeight);
	float GetDistance(const FVector& StartLocation, const FVector& EndLocation, const FVector& Normal);

	//===== Basic Trace Logic =====//
	FHitResult SphereTrace(const FVector& Start, const FVector& End, float Radius, ECollisionChannel TraceChannel, bool bDrawDebug) const;
	FHitResult BoxTrace(const FVector& Start, const FVector& End, FVector BoxHalfSize, FRotator Rotation, bool bDrawDebug) const;
	FHitResult LineTrace(const FVector& Start, const FVector& End, bool bDrawDebug) const;
	FHitResult CapsuleTrace(const FVector& Start, const FVector& End, float Radius, float HalfHeight, bool bDrawDebug) const;
	void DrawSphereTrace(const FVector& Center, float Radius, float LifeTime) const;


};
