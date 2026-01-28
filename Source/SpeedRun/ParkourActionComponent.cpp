// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourActionComponent.h"
#include "SpeedRunCharacter.h"
#include "SpeedRunPlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values for this component's properties
UParkourActionComponent::UParkourActionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UParkourActionComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<ASpeedRunCharacter>(GetOwner());

	if (Player)
	{
		PlayerController = Cast<ASpeedRunPlayerController>(Player->GetController());
	}	
}


// Called every frame
void UParkourActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UParkourActionComponent::SetupParkourInputComponent(UEnhancedInputComponent* EnhancedInputComponent)
{
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(LedgeHangUpAction, ETriggerEvent::Started, this, &UParkourActionComponent::HandleLedgeHangUp);
		EnhancedInputComponent->BindAction(LedgeShimmyAction, ETriggerEvent::Triggered, this, &UParkourActionComponent::HandleLedgeShimmy);
		EnhancedInputComponent->BindAction(LedgeDropAction, ETriggerEvent::Started, this, &UParkourActionComponent::HandleLedgeDrop);
		EnhancedInputComponent->BindAction(ParkourJumpAction, ETriggerEvent::Started, this, &UParkourActionComponent::HandleParkourJump);
	}
}

void UParkourActionComponent::HandleLedgeHangUp(const FInputActionValue& Value)
{
}

void UParkourActionComponent::HandleLedgeShimmy(const FInputActionValue& Value)
{
}

void UParkourActionComponent::HandleLedgeDrop(const FInputActionValue& Value)
{
}

void UParkourActionComponent::HandleParkourJump(const FInputActionValue& Value)
{
}

bool UParkourActionComponent::CanParkourJump(const FInputActionValue& Value)
{
	return false;
}

void UParkourActionComponent::DetectAnything()
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

            /*
            FDetectWallInfo WallInfo;

            WallInfo.bHit = HitResult.bBlockingHit;
            WallInfo.HitLocation = HitResult.Location;
            WallInfo.HitNormal = HitResult.Normal;

            return WallInfo;*/
        }

    }
}


FVector UParkourActionComponent::MoveVectorUpward(FVector Vector, float ZOffset)
{
    return Vector + FVector(0.f, 0.f, ZOffset);
}

FVector UParkourActionComponent::MoveVectorDownward(FVector Vector, float ZOffset)
{
    return Vector - FVector(0.f, 0.f, ZOffset);;
}

FVector UParkourActionComponent::MoveVectorForward(FVector Vector, FRotator Rotation, float Distance)
{
    FVector ForwardVector = UKismetMathLibrary::GetForwardVector(Rotation);

    return Vector + (ForwardVector * Distance);
}

FVector UParkourActionComponent::MoveVectorBackward(FVector Vector, FRotator Rotation, float Distance)
{
    FVector ForwardVector = UKismetMathLibrary::GetForwardVector(Rotation);

    return Vector - (ForwardVector * Distance);
}

FVector UParkourActionComponent::MoveVectorLeft(FVector Vector, FRotator Rotation, float Distance)
{
    FVector RightVector = UKismetMathLibrary::GetRightVector(Rotation);

    return Vector - (RightVector * Distance);
}

FVector UParkourActionComponent::MoveVectorRight(FVector Vector, FRotator Rotation, float Distance)
{
    FVector RightVector = UKismetMathLibrary::GetRightVector(Rotation);

    return Vector + (RightVector * Distance);
}

FRotator UParkourActionComponent::ReverseNormal(FVector Normal)
{
    FRotator RotFromX = UKismetMathLibrary::MakeRotFromX(Normal);

    return UKismetMathLibrary::NormalizedDeltaRotator(RotFromX, FRotator(0.f, 180.f, 0.f));
}

void UParkourActionComponent::TryParkourAction()
{
}

UParkourDataAsset* UParkourActionComponent::GetBestActionMatch()
{
    return nullptr;
}

