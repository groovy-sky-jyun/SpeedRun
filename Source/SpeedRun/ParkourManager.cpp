// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourManager.h"
#include "SpeedRunCharacter.h"
#include "ParkourAction.h"
#include "SpeedRunPlayerController.h"


UParkourManager::UParkourManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UParkourManager::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<ASpeedRunCharacter>(GetOwner());

	for (UParkourAction* Action : ActionList)
	{
		if (Action)
		{
			Action->Initialize(Player, this);

			InputActionMap.FindOrAdd(Action->GetInputType()).Add(Action);
		}
	}

	CurrentStateType = EParkourStateType::None;
}

void UParkourManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


bool UParkourManager::TryDetectParkour()
{

	if (true)// parkour 조건 만족 시
	{
		SwitchToParkourInput(true);
	}

	return false;
}

void UParkourManager::SwitchToParkourInput(bool Value)
{
	if (ASpeedRunPlayerController* PC = Cast<ASpeedRunPlayerController>(Player->GetController()))
	{
		PC->UpdateParkourMappingContext(Value);
	}
}


void UParkourManager::SetupParkourInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &UParkourManager::HandleMove);
		EnhancedInputComponent->BindAction(UpAction, ETriggerEvent::Started, this, &UParkourManager::HandleUp_Start);
		EnhancedInputComponent->BindAction(UpAction, ETriggerEvent::Completed, this, &UParkourManager::HandleUp_End);
		EnhancedInputComponent->BindAction(DownAction, ETriggerEvent::Started, this, &UParkourManager::HandleDown);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &UParkourManager::HandleDash);
	}
}

void UParkourManager::HandleMove(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	//DoMove(MovementVector.X, MovementVector.Y);
}


void UParkourManager::HandleUp_Start(const FInputActionValue& Value)
{
	UParkourAction* NewAction = CheckPlayAction(EParkourStateType::Up);

	if (NewAction != nullptr)
	{
		PlayAction(NewAction);
	}
	else
	{
		Player->Jump();

		NewAction = CheckPlayAction(EParkourStateType::Up);

		if (NewAction != nullptr)
		{
			PlayAction(NewAction);
		}
	}	
}

void UParkourManager::HandleUp_End(const FInputActionValue& Value)
{
	if (CurrentAction == nullptr)
	{
		Player->StopJumping();
	}

	CurrentAction = nullptr;
}

void UParkourManager::HandleDown(const FInputActionValue& Value)
{
	UParkourAction* NewAction = CheckPlayAction(EParkourStateType::Down);

	if (NewAction != nullptr)
	{
		PlayAction(NewAction);
	}
	
}

void UParkourManager::HandleDash(const FInputActionValue& Value)
{
	UParkourAction* NewAction = CheckPlayAction(EParkourStateType::Sprint);

	if (NewAction != nullptr)
	{
		PlayAction(NewAction);
	}
}


UParkourAction* UParkourManager::CheckPlayAction(EParkourStateType InputType)
{
	// 현재 상태에서 action을 변경(연계나 취소)할 수 있는지 확인
	
	if (TArray<UParkourAction*>* InputActionList = InputActionMap.Find(InputType))
	{
		for (UParkourAction* Action : *InputActionList)
		{
			// 우선순위대로 실행 조건 체크
			if (Action && Action->CheckVisibleToAction())
			{
				return Action;
			}
		}
	}

	return nullptr;
}

void UParkourManager::PlayAction(UParkourAction* NewAction)
{
	if (CurrentAction)
	{
		CurrentAction->OnEnd();	
	}

	CurrentAction = NewAction;

	NewAction->OnStart();
}

void UParkourManager::UpdateState(EParkourStateType InputType)
{
	CurrentStateType = InputType;
}
