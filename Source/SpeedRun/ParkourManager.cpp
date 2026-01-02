// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourManager.h"
#include "SpeedRunCharacter.h"
#include "ParkourAction.h"

// Sets default values for this component's properties
UParkourManager::UParkourManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
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


// Called every frame
void UParkourManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UParkourManager::SetupParkourInputComponent(UEnhancedInputComponent* ParkourInputComponent)
{
	ParkourInputComponent->BindAction(UpAction, ETriggerEvent::Started, this, &UParkourManager::Input_Up_Start);
	ParkourInputComponent->BindAction(UpAction, ETriggerEvent::Completed, this, &UParkourManager::Input_Up_End);

	ParkourInputComponent->BindAction(DownAction, ETriggerEvent::Started, this, &UParkourManager::Input_Down);

	ParkourInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &UParkourManager::Sprint);

	ParkourInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &UParkourManager::Interaction);
}

void UParkourManager::Input_Up_Start(const FInputActionValue& Value)
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

	/*
	UpdateState(EParkourStateType::Up);

	if (CurrentAction == nullptr)
	{
		Player->Jump();
	}
	else
	{
		CheckPlayAction(EParkourStateType::Up);
	}*/

	
}

void UParkourManager::Input_Up_End(const FInputActionValue& Value)
{
	if (CurrentAction == nullptr)
	{
		Player->StopJumping();
	}

	CurrentAction = nullptr;
}

void UParkourManager::Input_Down(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Input_Down"));

	CheckPlayAction(EParkourStateType::Down);
}

void UParkourManager::Sprint(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Sprint"));

	CheckPlayAction(EParkourStateType::Sprint);
}

void UParkourManager::Interaction()
{
	UE_LOG(LogTemp, Warning, TEXT("Interaction"));

	CheckPlayAction(EParkourStateType::Interact);

	// 해당 액터 Interface 작성해서 공통적으로 함수가지게 하고
	// 각 액터마다 다르게 로직 실행되도록 구현
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


/*
* Variables Get/Set Function
*/
bool UParkourManager::GetCanMove()
{
	return bCanMove;
}

void UParkourManager::SetCanMove(bool Value)
{
	bCanMove = Value;
}

bool UParkourManager::GetIsOnLedge()
{
	return bIsOnLedge;
}

void UParkourManager::SetIsOnLedge(bool Value)
{
	bIsOnLedge = Value;
}

bool UParkourManager::GetLedgeHasFootSurfaceR()
{
	return bLedgeHasFootSurfaceR;
}

void UParkourManager::SetLedgeHasFootSurfaceR(bool Value)
{
	bLedgeHasFootSurfaceR = Value;
}

bool UParkourManager::GetLedgeHasFootSurfaceL()
{
	return bLedgeHasFootSurfaceL;
}

void UParkourManager::SetLedgeHasFootSurfaceL(bool Value)
{
	bLedgeHasFootSurfaceL = Value;
}

bool UParkourManager::GetOverrideFootIK()
{
	return bOverrideFootIK;
}

void UParkourManager::SetOverrideFootIK(bool Value)
{
	bOverrideFootIK = Value;
}
