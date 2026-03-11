// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourBlock.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ParkourComponent.h"
#include "Math/UnrealMathUtility.h"

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

void AParkourBlock::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // 1. Static Mesh의 크기 자동 조절 
    BlockSize.X = FMath::Max(BlockSize.X, 1.0);
    BlockSize.Y = FMath::Max(BlockSize.Y, 1.0);
    BlockSize.Z = FMath::Max(BlockSize.Z, 1.0);

    // (기본 큐브가 100x100x100 기준이므로, BlockSize를 100으로 나눈 값이 Scale)
    StaticMesh->SetRelativeScale3D(BlockSize / 100.f);

    float MaxX = BlockSize.X;
    float MaxY = BlockSize.Y;
    float MaxZ = BlockSize.Z;

    // 3. 각 스플라인의 위치를 큐브 모서리에 정확히 맞춤
    auto SetupLedge = [](USplineComponent* Spline, FVector StartPos, FVector EndPos)
    {
        if (Spline)
        {
            Spline->ClearSplinePoints(true);
            Spline->AddSplinePoint(StartPos, ESplineCoordinateSpace::Local, true);
            Spline->AddSplinePoint(EndPos, ESplineCoordinateSpace::Local, true);
            Spline->SetSplinePointType(0, ESplinePointType::Linear, true);
            Spline->SetSplinePointType(1, ESplinePointType::Linear, true);
        }
    };

    // 앞면 (Y=0 모서리 / X가 0에서 MaxX로 진행)
    SetupLedge(Ledge_Front, FVector(0.f, 0.f, MaxZ), FVector(MaxX, 0.f, MaxZ));

    // 뒷면 (Y=MaxY 모서리 / X가 MaxX에서 0으로 진행)
    SetupLedge(Ledge_Back, FVector(MaxX, MaxY, MaxZ), FVector(0.f, MaxY, MaxZ));

    // 왼쪽 (X=0 모서리 / Y가 MaxY에서 0으로 진행)
    SetupLedge(Ledge_Left, FVector(0.f, MaxY, MaxZ), FVector(0.f, 0.f, MaxZ));

    // 오른쪽 (X=MaxX 모서리 / Y가 0에서 MaxY로 진행)
    SetupLedge(Ledge_Right, FVector(MaxX, 0.f, MaxZ), FVector(MaxX, MaxY, MaxZ));
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
        FVector Location = Ledges[i]->FindLocationClosestToWorldLocation(ActorLocation, ESplineCoordinateSpace::World);
        FVector UpLocation = Ledges[i]->FindUpVectorClosestToWorldLocation(ActorLocation, ESplineCoordinateSpace::World);

        float Distance = FVector::Dist(Location + (UpLocation * 10), ActorLocation);
        if (Distance < ClosestDistance || i == 0)
        {
            ClosestDistance = Distance;
            ClosestIndex = i;
        }
    }

    if(ClosestIndex == -1) return nullptr;
    return Ledges[ClosestIndex];
}



FTraversalCheckResult AParkourBlock::GetLedgeTransform(FVector HitLocation, FVector ActorLocation)
{
    FTraversalCheckResult CheckResult;

    USplineComponent* ClosestLedge = FindLedgeClosestToActor(ActorLocation);
    if (!ClosestLedge || ClosestLedge->GetSplineLength() < MinLedgeWidth)
    {
        CheckResult.Obstacle_Data.bHasFrontLedge = false;
        return CheckResult;
    }

    // 1.Save FrontLedge Data 
    // 1.1.Spline에서 HitLocation와 가장 가까운 위치(점) 찾기
    FVector LocalClosestPoint = ClosestLedge->FindLocationClosestToWorldLocation(HitLocation, ESplineCoordinateSpace::Local);
    // 1.2.특정 좌표가 Spline 시작점으로 부터 몇 cm 떨어져 있는지 계산
    float Distance = ClosestLedge->GetDistanceAlongSplineAtLocation(LocalClosestPoint, ESplineCoordinateSpace::Local);
    
    // 1.3.Spline 안에 플레이어 손이 위치할 수 있도록 위치 보정
    float MinWidth = MinLedgeWidth / 2.f;
    float MaxWidth = ClosestLedge->GetSplineLength() - MinWidth;
    float ClampedDistance = FMath::Clamp(Distance, MinWidth, MaxWidth);

    // 1.4.FTraversalCheckResult에다가 데이터 저장
    FTransform SplineTransform = ClosestLedge->GetTransformAtDistanceAlongSpline(ClampedDistance, ESplineCoordinateSpace::World);
    CheckResult.Obstacle_Data.bHasFrontLedge = true;
    CheckResult.Obstacle_Data.FrontLedgeLocation = SplineTransform.GetLocation();
    FVector FrontSplineForward = SplineTransform.GetRotation().GetForwardVector();
    FVector FrontLedgeNormal = FrontSplineForward.RotateAngleAxis(-90.0f, FVector::UpVector); // Z축 기준으로 -90도 회전
    CheckResult.Obstacle_Data.FrontLedgeNormal = FrontLedgeNormal;
 

    // 2.Save BackLedge Data 
    TObjectPtr<USplineComponent>* BackSplinePtr = OppositeLedges.Find(ClosestLedge);
    if(BackSplinePtr == nullptr)
    {
        CheckResult.Obstacle_Data.bHasBackLedge = false;
        return CheckResult;
    }

    USplineComponent* BackSpline = *BackSplinePtr;
    FTransform BackSplineTransform = BackSpline->FindTransformClosestToWorldLocation(CheckResult.Obstacle_Data.FrontLedgeLocation, ESplineCoordinateSpace::World);
    CheckResult.Obstacle_Data.bHasBackLedge = true;
    CheckResult.Obstacle_Data.BackLedgeLocation = BackSplineTransform.GetLocation();
    
    FVector BackSplineForward = BackSplineTransform.GetRotation().GetForwardVector();
    FVector BackLedgeNormal = BackSplineForward.RotateAngleAxis(-90.0f, FVector::UpVector); // Z축 기준으로 -90도 회전
    CheckResult.Obstacle_Data.BackLedgeNormal = BackLedgeNormal;



    return CheckResult;
}


FTraversalCheckResult AParkourBlock::GetLedgeTransformToStepBox(FVector HitLocation)
{
    FTraversalCheckResult CheckResult;

    USplineComponent* ClosestLedge = FindLedgeClosestToActor(HitLocation);
    if (!ClosestLedge || ClosestLedge->GetSplineLength() < StepBoxMinLedgeWidth)
    {
        CheckResult.StepBox_Data.bHasNextFrontLedge = false;
        return CheckResult;
    }

    //Spline에서 HitLocation와 가장 가까운 위치(점) 찾기
    FVector LocalClosestPoint = ClosestLedge->FindLocationClosestToWorldLocation(HitLocation, ESplineCoordinateSpace::Local);
    //특정 좌표가 Spline 시작점으로 부터 몇 cm 떨어져 있는지 계산
    float Distance = ClosestLedge->GetDistanceAlongSplineAtLocation(LocalClosestPoint, ESplineCoordinateSpace::Local);

    //Spline 안에 플레이어 발이 위치할 수 있도록 위치 보정
    float MinWidth = StepBoxMinLedgeWidth / 2.f;
    float MaxWidth = ClosestLedge->GetSplineLength() - MinWidth;
    float ClampedDistance = FMath::Clamp(Distance, MinWidth, MaxWidth);

    FTransform SplineTransform = ClosestLedge->GetTransformAtDistanceAlongSpline(ClampedDistance, ESplineCoordinateSpace::World);
    CheckResult.StepBox_Data.bHasNextFrontLedge = true;
    CheckResult.StepBox_Data.NextFrontLedgeLocation = SplineTransform.GetLocation();
    CheckResult.StepBox_Data.NextFrontLedgeNormal = SplineTransform.GetRotation().GetUpVector();

    return CheckResult;
}