// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourAction.h"
#include "SpeedRunCharacter.h"
#include "ParkourMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "ParkourManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"


void UParkourAction::Initialize(ASpeedRunCharacter* OwnerPlayer, UParkourManager* ParkourComponent)
{
    Player = OwnerPlayer;
    ParkourManager = ParkourComponent;

    if (Player)
    {
		Movement = Cast<UParkourMovementComponent>(Player->GetCharacterMovement());
		Capsule = Player->GetCapsuleComponent();
        WarpComponent = Player->GetMotionWarpingComponent();
        CapsuleComponent = Player->GetCapsuleComponent();
        AnimInstance = Player->GetMesh()->GetAnimInstance();

		if (Capsule)
		{
			CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}
    }
}

const UWorld* UParkourAction::GetPlayerWorld()
{
    if (GetOuter())
    {
        return GetOuter()->GetWorld();
    }

    return nullptr;
}

FVector UParkourAction::MoveVectorUpward(FVector Vector, float ZOffset)
{
    return Vector + FVector(0.f, 0.f, ZOffset);
}

FVector UParkourAction::MoveVectorDownward(FVector Vector, float ZOffset)
{
    return Vector - FVector(0.f, 0.f, ZOffset);
}

FVector UParkourAction::MoveVectorForward(FVector Vector, FRotator Rotation, float Distance)
{
    FVector ForwardVector = UKismetMathLibrary::GetForwardVector(Rotation);

    return Vector + (ForwardVector * Distance);
}

FVector UParkourAction::MoveVectorBackward(FVector Vector, FRotator Rotation, float Distance)
{
    FVector ForwardVector = UKismetMathLibrary::GetForwardVector(Rotation);

    return Vector - (ForwardVector * Distance);
}

FVector UParkourAction::MoveVectorLeft(FVector Vector, FRotator Rotation, float Distance)
{
    FVector RightVector = UKismetMathLibrary::GetRightVector(Rotation);

    return Vector - (RightVector * Distance);
}

FVector UParkourAction::MoveVectorRight(FVector Vector, FRotator Rotation, float Distance)
{
    FVector RightVector = UKismetMathLibrary::GetRightVector(Rotation);

    return Vector + (RightVector * Distance);
}

FRotator UParkourAction::ReverseNormal(FVector Normal)
{
    FRotator RotFromX = UKismetMathLibrary::MakeRotFromX(Normal);

    return UKismetMathLibrary::NormalizedDeltaRotator(RotFromX, FRotator(0.f, 180.f, 0.f));
}

FDetectWallInfo UParkourAction::DetectWall()
{
    for (int i = 0; i < 7; i++)
    {
        FVector CenterLocation = MoveVectorUpward(MoveVectorDownward(Player->GetActorLocation(), 60.f), i * 20.f);

        FVector StartLocation = MoveVectorBackward(CenterLocation, Player->GetActorRotation(), 30.f);
        FVector EndLocation = CenterLocation + Player->GetActorForwardVector() * 200.f;


        FHitResult HitResult;
        float Radius = 10.f;
        TArray<AActor*> ActorsToIgnore;
        ActorsToIgnore.Add(Player);

        bool bDetectWall = UKismetSystemLibrary::SphereTraceSingle(
            GetWorld(),
            StartLocation,
            EndLocation,
            Radius,
            UEngineTypes::ConvertToTraceType(ECC_Visibility), // Trace Channel
            false,         // Trace Complex
            ActorsToIgnore,
            EDrawDebugTrace::ForDuration, // Draw Debug Type
            HitResult,
            true,          // Ignore Self
            FLinearColor::Red,   // 디버그 선 색상
            FLinearColor::Green, // 히트 시 색상
            5.0f           // 디버그 선 유지 시간
        );


        if (bDetectWall)
        {
            FDetectWallInfo WallInfo;

            WallInfo.bHit = HitResult.bBlockingHit;
            WallInfo.HitLocation = HitResult.Location;
            WallInfo.HitNormal = HitResult.Normal;
            
            return WallInfo;
        }

    }
    
    return FDetectWallInfo();
}
