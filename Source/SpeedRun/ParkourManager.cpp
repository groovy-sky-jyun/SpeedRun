// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourManager.h"
#include "SpeedRunCharacter.h"
#include "ParkourAction.h"
#include "SpeedRunPlayerController.h"
#include "ParkourHangUp.h"
#include "ParkourDrop.h"
#include "ParkourHang.h"
#include "ParkourShimmy.h"


UParkourManager::UParkourManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UParkourManager::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<ASpeedRunCharacter>(GetOwner());

	if (Player)
	{
		PlayerController = Cast<ASpeedRunPlayerController>(Player->GetController());
	}

	for (UParkourAction* Action : ActionList)
	{
		if (Action)
		{
			Action->Initialize(Player, this);
		}
	}
}

void UParkourManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UParkourManager::SetupParkourInputComponent(UEnhancedInputComponent* EnhancedInputComponent)
{
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(LedgeHangUpAction, ETriggerEvent::Started, this, &UParkourManager::HandleLedgeHangUp);
		EnhancedInputComponent->BindAction(LedgeShimmyAction, ETriggerEvent::Triggered, this, &UParkourManager::HandleLedgeShimmy);
		EnhancedInputComponent->BindAction(LedgeDropAction, ETriggerEvent::Started, this, &UParkourManager::HandleLedgeDrop);
		EnhancedInputComponent->BindAction(ParkourJumpAction, ETriggerEvent::Started, this, &UParkourManager::HandleParkourJump);
	}
}


void UParkourManager::HandleLedgeHangUp(const FInputActionValue& Value)
{
	for (auto* ParkourAction : ActionList)
	{
		if (UParkourHangUp* ParkourHangUp = Cast<UParkourHangUp>(ParkourAction))
		{
			if (ParkourHangUp->CheckVisibleToAction())
			{
				CurrentAction = ParkourHangUp;
				ParkourHangUp->OnStart();
			}
			
		}
	}
}

void UParkourManager::HandleLedgeShimmy(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	// find out which way is forward
	const FRotator Rotation = PlayerController->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get right vector 
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// add movement
	for (auto* ParkourAction : ActionList)
	{
		if (UParkourShimmy* ParkourShimmy = Cast<UParkourShimmy>(ParkourAction))
		{
			if (ParkourShimmy->CheckVisibleToAction(RightDirection, MovementVector.X))
			{
				CurrentAction = ParkourShimmy;
				ParkourShimmy->OnStart();
			}
		}
	}
}


void UParkourManager::HandleLedgeDrop(const FInputActionValue& Value)
{
	for (auto* ParkourAction : ActionList)
	{
		if (UParkourDrop* ParkourDrop = Cast<UParkourDrop>(ParkourAction))
		{
			if (ParkourDrop->CheckVisibleToAction())
			{
				CurrentAction = ParkourDrop;
				ParkourDrop->OnStart();
				PlayerController->UpdateParkourMappingContext(false);
			}
		}
	}
}

void UParkourManager::HandleParkourJump(const FInputActionValue& Value)
{
	for (auto* ParkourAction : ActionList)
	{
		if (UParkourHang* ParkourHang = Cast<UParkourHang>(ParkourAction))
		{
			if (ParkourHang->CheckVisibleToAction())
			{
				CurrentAction = ParkourHang;
				ParkourHang->OnStart();
				PlayerController->UpdateParkourMappingContext(true);
			}
		}
	}

}

bool UParkourManager::CanParkourJump(const FInputActionValue& Value)
{
	for (auto* ParkourAction : ActionList)
	{
		if (UParkourHang* ParkourHang = Cast<UParkourHang>(ParkourAction))
		{
			if (ParkourHang->CheckVisibleToAction())
			{
				ParkourHang->OnStart();
				PlayerController->UpdateParkourMappingContext(true);

				return true;
			}
			return false;
		}
	}
	return false;
}


/* 조건 만족하는 Action 있다면 실행
bool UParkourManager::TryNextParkourAction(EInputType InputType)
{
	if (UParkourAction* ParkourAction = FindNextAction(InputType))
	{
		OnStartParkourAction(ParkourAction);

		return true;
	}

	return false;
}

/* 실행 조건이 만족되는 다음 Action 찾기
UParkourAction* UParkourManager::FindNextAction(EInputType InputType)
{
	if (TArray<UParkourAction*>* InputActionList = InputActionMap.Find(InputType))
	{
		for (UParkourAction* NewAction : *InputActionList)
		{
			// 우선순위대로 실행 조건 체크
			if (NewAction && NewAction->CheckVisibleToAction())
			{
				return NewAction;
			}
		}
	}

	return nullptr;
}
*/

void UParkourManager::OnStartParkourAction(UParkourAction* NewAction)
{
	if (CurrentAction)
	{
		CurrentAction->OnEnd();
	}

	CurrentAction = NewAction;

	if (PlayerController)
	{
		if (!PlayerController->HasParkourIMC())
		{
			PlayerController->UpdateParkourMappingContext(true);
		}
	}

	NewAction->OnStart();
}

void UParkourManager::OnEndParkourAction()
{
	if (CurrentAction)
	{
		CurrentAction->OnEnd();
	}

	CurrentAction = nullptr;

	if (PlayerController)
	{
		if (PlayerController->HasParkourIMC())
		{
			PlayerController->UpdateParkourMappingContext(false);
		}
	}
}
