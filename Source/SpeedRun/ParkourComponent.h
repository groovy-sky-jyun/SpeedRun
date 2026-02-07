// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "ParkourComponent.generated.h"

class ASpeedRunCharacter;
class UParkourMovementComponent;
class UMotionWarpingComponent;
class UParkourDataAsset;
class UDA_EnvironmentTags;
class UDA_ParkourActionCategory;
class UDA_JumpAction; 
struct FJumpOption;

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
	void DoParkourJump();


	
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
	FGameplayTagContainer CurrenActionTags;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	FGameplayTagContainer CurrenEnvironmentTags;
	

	//===== DataAsset =====//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataAsset")
	TArray<TObjectPtr<UDA_EnvironmentTags>> DA_EnvironmentTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataAsset")
	TArray<TObjectPtr<UDA_ParkourActionCategory>> ActionCategoryList;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DataAsset")
	TObjectPtr<UDA_JumpAction> JumpOptionList;



	//===== GameplayTag =====//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag|Obstacle")
	FGameplayTag Tag_DetectObstacle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag|Obstacle")
	FGameplayTag Tag_DetectNone;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag|Category")
	FGameplayTag TagCategory_SurfaceGap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag|Category")
	FGameplayTag TagCategory_ObstacleHeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag|Category")
	FGameplayTag TagCategory_Jump;


	//===== Trace Setting Value =====//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Detect")
	float DetectZOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Detect")
	float DetectDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Detect")
	float DetectWidth_Gap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Detect")
	float DetectWidth_Count;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Detect")
	float DetectWidth_Radius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|Detect")
	float DetectHeight_ZOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|SurfaceGap")
	float SurfaceGap_Height;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|SurfaceGap")
	int32 SurfaceGap_Count;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|SurfaceGap")
	float SurfaceGap_Gap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|SurfaceGap")
	float SurfaceGap_Radius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config|SurfaceGap")
	float SurfaceGap_Distance;


	//===== Animation Setting Value =====//
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionWarping")
	//float MotionWarpingZOffset;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionWarping")
	//float MotionWarpingDistance;


private:
	//===== Tag 결정 =====//
	FGameplayTag FindCurrentActionTagForParentTag(FGameplayTag ParentTag) const;
	void AddNewEnvironmentTag(FGameplayTag TagCategory, float Value);
	FGameplayTag FindActionTagByEnvironmentTags(FGameplayTag ActionCategory) const;
	FJumpOption GetJumpOption(FGameplayTag NewTag) const;


	//===== 장애물 및 환경 감지 (Trace Logic) =====//
	FHitResult IsDetectObstacle();
	float GetObstacleHeightValue(const FHitResult& DetectHitResult);
	float GetObstacleGapValue(const FHitResult& DetectHitResult);
	float GetSurfaceGapValue();
	FHitResult DetectToVerticalTraces(int32 TraceCount, float Gap, float Distance, FVector Start, FVector TraceDir, float Radius, bool bReturnHit) const;
	FHitResult DetectToHorizontalTraces(int32 TraceCount, float Gap, float Distance, FVector Start, FVector TraceDir, float Radius, bool bReturnHit) const;
	bool SphereTrace(FHitResult& HitResult, const FVector& Start, const FVector& End, float Radius) const;
	bool LineTrace(FHitResult& HitResult, const FVector& Start, const FVector& End) const;
};
