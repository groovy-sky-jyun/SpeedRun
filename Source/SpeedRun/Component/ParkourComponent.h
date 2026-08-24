#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "ParkourComponent.generated.h"

class ASpeedRunCharacter;
class UParkourMovementComponent;
class UMotionWarpingComponent;
class AParkourBlock;
class UAnimInstance;
class UParkourActionBase;

USTRUCT(BlueprintType)
struct FObstacleData : public FTableRowBase
{
	GENERATED_BODY()

	// FrontLedge Data
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasFrontLedge = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector FrontLedgeLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector FrontLedgeNormal = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float FrontHeight = 0.f;

	// BackLedge Data
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasBackLedge = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector BackLedgeLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector BackLedgeNormal = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Depth = 0.f;

	// UpperSurface Data
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasUpperSurface = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector UpperSurfaceLocation = FVector::ZeroVector;

	// LandingSurface Data
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasLandingSurface = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector LandingSurfaceLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float BackDropHeight = 0.f;
};

USTRUCT(BlueprintType)
struct FEnvData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag HitParkourTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FObstacleData Obstacle_Data;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UPrimitiveComponent> HitComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CurrentSpeed = 0.f;
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
	UFUNCTION(BlueprintCallable, Category = "Parkour")
	void PerformJumpSequence();

	UParkourActionBase* EvaluateNextAction(const FEnvData& EnvData);

	//===== Action State =====//
	UFUNCTION(BlueprintCallable, Category = "Action|State")
	void DropFromHang();

	UFUNCTION(BlueprintCallable, Category = "Action|State")
	void EnterHangState();

	void AlignToLedge(const FEnvData& EnvData);

	//===== Motion Warping =====//
	void AddWarpTarget(FName TargetName, FVector Location, FVector Normal);
	void ClearAllWarpTargets();

public:
	//===== Property =====//
	UPROPERTY(VisibleAnywhere, Category = "Parkour|State")
	bool bIsHanging = false;

	// BackLedge 에서 뒤쪽 바닥까지의 낙차 최소 거리
	UPROPERTY(EditAnywhere, Category = "Parkour|Config")
	float MinHeightBlock = 50.f;

	// 파쿠르 판정 트레이스를 화면에 그릴지 여부.
	// 켜면 스페이스바를 누를 때 감지 구체 / 캡슐 경로 / 렛지 마커가 보인다.
	UPROPERTY(EditAnywhere, Category = "Parkour|Debug")
	bool bShowDebugTrace = false;

	UPROPERTY(EditAnywhere, Instanced, Category = "Parkour|Actions")
	TArray<UParkourActionBase*> RegisteredActions;

	UPROPERTY()
	FEnvData CurrentEnvData;


private:
	//===== Detect Environment =====//
	bool TryUpdateEnvData(FEnvData& EnvData);
	FHitResult TryDetectObstacle(FVector ActorLocation, FVector ActorForward);
	bool UpdateObstacleData(FHitResult ObstacleHitResult, AParkourBlock* Block, FEnvData& EnvData, FVector ActorLocation);
	bool CanLanding(const FEnvData& EnvData, FVector& LandingLocation, float& DropHeight);
	float GetDistance(const FVector& StartLocation, const FVector& EndLocation, const FVector& Normal);

	//===== Basic Trace Logic =====//
	FHitResult SphereTrace(const FVector& Start, const FVector& End, float Radius, ECollisionChannel TraceChannel, bool bDrawDebug) const;
	FHitResult CapsuleTrace(const FVector& Start, const FVector& End, float Radius, float HalfHeight, bool bDrawDebug) const;
	void DrawSphereTrace(const FVector& Center, float Radius, float LifeTime) const;


private:
	//===== 참조 및 상태 변수 =====//
	UPROPERTY()
	TObjectPtr<ASpeedRunCharacter> Player;

	UPROPERTY()
	TObjectPtr<UAnimInstance> AnimInstance;

	UPROPERTY()
	TObjectPtr<UMotionWarpingComponent> WarpComponent;

	UPROPERTY()
	TObjectPtr<class UParkourMovementComponent> ParkourMovement;

	float CapsuleRadius;
	float CapsuleHalfHeight;


/*
private:
	FHitResult BoxTrace(const FVector& Start, const FVector& End, FVector BoxHalfSize, FRotator Rotation, bool bDrawDebug) const;
	FHitResult LineTrace(const FVector& Start, const FVector& End, bool bDrawDebug) const;
	*/
};
