// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "ParkourComponent.h"
#include "ParkourBlock.generated.h"

struct FTraversalCheckResult;

UCLASS()
class SPEEDRUN_API AParkourBlock : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AParkourBlock();
	virtual void OnConstruction(const FTransform& Transform) override;
	 
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Mesh")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spline")
	TObjectPtr<USplineComponent> Ledge_Front;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spline")
	TObjectPtr<USplineComponent> Ledge_Back;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spline")
	TObjectPtr<USplineComponent> Ledge_Right;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spline")
	TObjectPtr<USplineComponent> Ledge_Left;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
	TArray<TObjectPtr<USplineComponent>> Ledges;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
	TMap<TObjectPtr<USplineComponent>, TObjectPtr<USplineComponent>> OppositeLedges;


public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour Settings", meta = (MakeEditWidget = true))
	FVector BlockSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	float MinLedgeWidth = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	float StepBoxMinLedgeWidth = 60.f;


public:
	UFUNCTION(BlueprintCallable)
	USplineComponent* FindLedgeClosestToActor(FVector ActorLocation);

	UFUNCTION(BlueprintCallable)
	FTraversalCheckResult GetLedgeTransform(FVector HitLocation, FVector ActorLocation);

	UFUNCTION(BlueprintCallable)
	FTraversalCheckResult GetLedgeTransformToStepBox(FVector HitLocation);
};
