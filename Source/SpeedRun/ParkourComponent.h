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
class UDA_TraceOptions;
class UDA_SphereTracesOption;
class UDA_BoxTraceOption;
class UDA_LineTraceOption;



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

	UFUNCTION(BlueprintCallable, Category = "Action")
	void DoLanding();
	
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
	//===== 장애물 및 환경 감지 =====//
	bool DetectObstacle();



	//===== Find DataAsset (feat.Tag) =====//
	FGameplayTag FindCurrentActionTagForParentTag(FGameplayTag ParentTag) const;
	void AddNewEnvironmentTag(FGameplayTag TagCategory, float Value);
	FGameplayTag FindActionTagByEnvironmentTags(FGameplayTag ActionCategory) const;
	FJumpOption GetJumpOption(FGameplayTag NewTag) const;



	//===== Basic Trace Logic =====//
	const UDA_SphereTracesOption* FindSpheresTraceOption(FGameplayTag TagCategory) const;
	const UDA_BoxTraceOption* FindBoxTraceOption(FGameplayTag TagCategory) const;
	const UDA_TraceOptions* FindLineTraceOption(FGameplayTag TagCategory) const;

	FHitResult DetectToHorTraces(FGameplayTag TagCategory, FVector Start, FVector Dir, bool bReturnHit, bool bDrawDebug) const;
	FHitResult DetectToVerTraces(FGameplayTag TagCategory, FVector Start, FVector Dir, bool bReturnHit, bool bDrawDebug) const;

	FHitResult SphereTrace(const FVector& Start, const FVector& End, float Radius, bool bDrawDebug) const;
	FHitResult BoxTrace(FGameplayTag TagCategory, FVector Start, FVector Dir, FRotator Rotation, bool bDrawDebug) const;
	FHitResult LineTraceVer(FGameplayTag TagCategory, FVector Start, FVector Dir, bool bDrawDebug) const;
	FHitResult LineTraceHor(FGameplayTag TagCategory, FVector Start, FVector Dir, bool bDrawDebug) const;
};
