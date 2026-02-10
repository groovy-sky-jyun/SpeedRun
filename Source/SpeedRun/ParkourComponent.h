// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Templates/Function.h"
#include "ParkourComponent.generated.h"

class ASpeedRunCharacter;
class UParkourMovementComponent;
class UMotionWarpingComponent;
class UParkourDataAsset;
class UDA_EnvironmentTags;
class UDA_ParkourActionCategory;
class UDA_JumpAction; 
struct FJumpOption;
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


	//===== 액션 수행 (Execution) =====//
	UFUNCTION(BlueprintCallable, Category = "Action")
	void TryParkourAction();

	UFUNCTION(BlueprintCallable, Category="Input")
	void HandleToJump();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void DoLanding();
	

protected:
	UFUNCTION(BlueprintCallable, Category = "Action")
	void SetupJumpPhysics();


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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	FGameplayTagContainer CurrentActionTags;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	FGameplayTagContainer CurrentEnvironmentTags;
	


	//===== DataAsset =====//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataAsset")
	TArray<TObjectPtr<UDA_EnvironmentTags>> DA_EnvironmentTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataAsset")
	TArray<TObjectPtr<UDA_ParkourActionCategory>> ActionCategoryList;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataAsset|Config")
	TObjectPtr<UDA_JumpAction> JumpConfigs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataAsset")
	TArray<TObjectPtr<UDA_SphereTracesOption>> SphereTracesOptions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataAsset")
	TArray<TObjectPtr<UDA_BoxTraceOption>> BoxTraceOptions;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataAsset")
	TArray<TObjectPtr<UDA_LineTraceOption>> LineTraceOptions;



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
	FGameplayTag Tag_Landing;



	//===== Animation Setting Value =====//
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionWarping")
	//float MotionWarpingZOffset;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionWarping")
	//float MotionWarpingDistance;


private:
	//===== Find DataAsset (feat.Tag) =====//
	FGameplayTag FindChildActionTag(const FGameplayTag& ParentTag) const;
	void UpdateEnvironmentTags(const FGameplayTag& TagCategory, float Value);
	void AddActionTag(const FGameplayTag& TagCategory);
	FGameplayTag SelectActionTagOnContext(const FGameplayTag& ActionCategory) const;
	FJumpOption FindJumpOption(const FGameplayTag& NewTag) const;


	//===== 장애물 및 환경 감지 =====//
	bool DetectObstacle();
	void MeasureObstacleDimensions();
	void AnalyzeEdgeEnvironment();
	void MeasureStepBoxWidth(const FVector& StepOverStart);
	void MeasureBuildingGap(const FVector& PlayerFootLocation);



	//===== Basic Trace Logic =====//
	template<typename T>
	const T* GetTraceDA(const TArray<TObjectPtr<T>>& TraceOptionArray, FGameplayTag TagCategory) const
	{
		for (auto& Option : TraceOptionArray)
		{
			if (Option->CategoryTag.MatchesTag(TagCategory))
			{
				return Option.Get();
			}
		}
		return nullptr;
	}
	const UDA_SphereTracesOption* GetTraceDA_Spheres(FGameplayTag TagCategory) const;
	const UDA_BoxTraceOption* GetTraceDA_Box(FGameplayTag TagCategory) const;
	const UDA_TraceOptions* GetTraceDA_Line(FGameplayTag TagCategory) const;

	FHitResult DetectSphereTraces(FGameplayTag TagCategory, FVector Start, FVector Dir, bool bReturnHit, ETraceDirection TraceDir, bool bDrawDebug) const;
	FHitResult SphereTrace(const FVector& Start, const FVector& End, float Radius, bool bDrawDebug) const;
	FHitResult BoxTrace(FGameplayTag TagCategory, FVector Start, FVector Dir, FRotator Rotation, bool bDrawDebug) const;
	FHitResult LineTraceVer(FGameplayTag TagCategory, FVector Start, FVector Dir, bool bDrawDebug) const;
	FHitResult LineTraceHor(FGameplayTag TagCategory, FVector Start, FVector Dir, bool bDrawDebug) const;
	FHitResult LineTrace(const FVector& Start, const FVector& End, bool bDrawDebug) const;
	
};
