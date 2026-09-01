// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpeedRunCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "SpeedRun.h"
#include "ParkourMovementComponent.h"
#include "MotionWarpingComponent.h" 
#include "GameplayTagContainer.h"
#include "ParkourComponent.h"
#include "Kismet/KismetSystemLibrary.h"

ASpeedRunCharacter::ASpeedRunCharacter(const FObjectInitializer& ObjectInitializer) : Super(
	ObjectInitializer.SetDefaultSubobjectClass<UParkourMovementComponent>(CharacterMovementComponentName))
{
	JumpMaxCount = 1;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Create Parkour Movement Component
	ParkourComponent = CreateDefaultSubobject<UParkourComponent>(TEXT("ParkourComponent"));

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarp"));
}

void ASpeedRunCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// 모든 컴포넌트가 세팅된 직후에 안전하게 캐스팅하여 저장
	ParkourMovementComponent = Cast<UParkourMovementComponent>(GetCharacterMovement());
}

void ASpeedRunCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASpeedRunCharacter::HandleLook);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASpeedRunCharacter::HandleMove);

		EnhancedInputComponent->BindAction(UpAction, ETriggerEvent::Started, this, &ASpeedRunCharacter::HandleUp);
		EnhancedInputComponent->BindAction(DownAction, ETriggerEvent::Started, this, &ASpeedRunCharacter::HandleDown);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &ASpeedRunCharacter::HandleDash);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ASpeedRunCharacter::HandleInteract);
	}
	else
	{
		UE_LOG(LogSpeedRun, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ASpeedRunCharacter::HandleLook(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ASpeedRunCharacter::HandleMove(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	DoMove(MovementVector.X, MovementVector.Y);
}

void ASpeedRunCharacter::HandleUp(const FInputActionValue& Value)
{
	DoUp();
}

void ASpeedRunCharacter::HandleDown(const FInputActionValue& Value)
{
	DoDown();
}

void ASpeedRunCharacter::HandleDash(const FInputActionValue& Value)
{
	DoDash();
}

void ASpeedRunCharacter::HandleInteract(const FInputActionValue& Value)
{
	// if 상호작용 물체에 focus 된 경우
	DoInteract();
}

void ASpeedRunCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ASpeedRunCharacter::DoMove(float Right, float Forward)
{
	if (!GetController()) return;

	if (ParkourComponent && ParkourComponent->IsHanging())
	{
		if (Forward > 0.5f)
		{
			ParkourComponent->PerformJumpSequence();
		}

		if (FMath::Abs(Right) > 0.01f)
		{
			AddMovementInput(GetActorRightVector(), Right);
		}
		return;
	}

	// find out which way is forward
	const FRotator Rotation = GetController()->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get forward vector
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	// get right vector 
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// add movement 
	AddMovementInput(ForwardDirection, Forward);
	AddMovementInput(RightDirection, Right);
}

void ASpeedRunCharacter::DoUp()
{
	// 매달림 여부와 무관하게 액션 평가에 맡긴다.
	// 매달려 있으면 Mantle 이 CUSTOM_Hang 을 보고 높은 점수를 반환해 올라서기가 선택된다.
	if (ParkourComponent)
	{
		ParkourComponent->PerformJumpSequence();
		return;
	}

	Jump();
}

void ASpeedRunCharacter::DoDown()
{
	if (ParkourComponent && ParkourComponent->IsHanging())
	{
		ParkourComponent->DropFromHang();
		return;
	}

	if (CanCrouch())
	{
		if (!GetCharacterMovement()->IsCrouching())
		{
			Crouch();
		}
		else
		{
			UnCrouch();
		}
		return;
	}
}

void ASpeedRunCharacter::DoDash()
{
	if (ParkourComponent && ParkourComponent->IsHanging()) return;

	FVector ForwardDir = GetActorForwardVector();
	LaunchCharacter(ForwardDir * DashDistance, true, true);
}

void ASpeedRunCharacter::DoInteract()
{
	UE_LOG(LogTemp, Warning, TEXT("Interact"));
}


