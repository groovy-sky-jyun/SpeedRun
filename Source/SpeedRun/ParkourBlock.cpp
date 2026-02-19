// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourBlock.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
AParkourBlock::AParkourBlock()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = DefaultSceneRoot;

    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    StaticMesh->SetupAttachment(DefaultSceneRoot);

    Ledge_Front = CreateDefaultSubobject<USplineComponent>(TEXT("Ledge_Front"));
    Ledge_Front->SetupAttachment(StaticMesh);

    Ledge_Back = CreateDefaultSubobject<USplineComponent>(TEXT("Ledge_Back"));
    Ledge_Back->SetupAttachment(StaticMesh);

    Ledge_Right = CreateDefaultSubobject<USplineComponent>(TEXT("Ledge_Right"));
    Ledge_Right->SetupAttachment(StaticMesh);

    Ledge_Left = CreateDefaultSubobject<USplineComponent>(TEXT("Ledge_Left"));
    Ledge_Left->SetupAttachment(StaticMesh);
}

// Called when the game starts or when spawned
void AParkourBlock::BeginPlay()
{
	Super::BeginPlay();
	
    Ledges.Empty();
    Ledges.Add(Ledge_Front);
    Ledges.Add(Ledge_Back);
    Ledges.Add(Ledge_Right);
    Ledges.Add(Ledge_Left);

    OppositeLedges.Empty();
    OppositeLedges.Add(Ledge_Front, Ledge_Back);
    OppositeLedges.Add(Ledge_Back, Ledge_Front);
    OppositeLedges.Add(Ledge_Right, Ledge_Left);
    OppositeLedges.Add(Ledge_Left, Ledge_Right);

}

USplineComponent* AParkourBlock::FindLedgeClosestToActor(FVector ActorLocation)
{
    if (Ledges.IsEmpty()) return nullptr;
   
    float ClosestDistance = 0.f;
    int32 ClosestIndex = -1;
    for (int32 i = 0; i < Ledges.Num(); i++)
    {
        FVector Loc = Ledges[i]->FindLocationClosestToWorldLocation(ActorLocation, ESplineCoordinateSpace::World);
        FVector UpLoc = Ledges[i]->FindUpVectorClosestToWorldLocation(ActorLocation, ESplineCoordinateSpace::World);

        float Distance = FVector::Dist(Loc + (UpLoc * 10), ActorLocation);
        if (Distance < ClosestDistance || i == 0)
        {
            ClosestDistance = Distance;
            ClosestIndex = i;
        }
    }

    if(ClosestIndex == -1) return nullptr;
    return Ledges[ClosestIndex];
}


