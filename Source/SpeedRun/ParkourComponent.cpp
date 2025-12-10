// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourComponent.h"
#include "SpeedRunCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

// Sets default values for this component's properties
UParkourComponent::UParkourComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	bCanWarp = false;
}


// Called when the game starts
void UParkourComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<ASpeedRunCharacter>(GetOwner());

	if (Player)
	{
		PlayerMovement = Player->GetCharacterMovement();

		TagContainer = Player->GetTagContainer();
	}
}


// Called every frame
void UParkourComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}


void UParkourComponent::SetupParkourInputComponent(UEnhancedInputComponent* ParkourInputComponent)
{
	ParkourInputComponent->BindAction(UpAction, ETriggerEvent::Started, this, &UParkourComponent::Input_Up_Start);
	ParkourInputComponent->BindAction(UpAction, ETriggerEvent::Completed, this, &UParkourComponent::Input_Up_End);

	ParkourInputComponent->BindAction(DownAction, ETriggerEvent::Started, this, &UParkourComponent::Input_Down);

	ParkourInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &UParkourComponent::Sprint);

	ParkourInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &UParkourComponent::Interaction);
}
//-----------------------


/* [Called when the Player Action Changes for input key ] */

void UParkourComponent::Input_Up_Start(const FInputActionValue& Value)
{
	if (TagContainer->HasTag(HangTag)) // 매달린 상태
	{
		if (TagContainer->HasTag(ShimmyTag))
		{
			ShimmyJump();
		}
		else if (TagContainer->HasTag(RopeTag))
		{
			RopeJump();
		}
		else
		{
			Mantle();
		}
		return;
	}
	

	if (TagContainer->HasTag(WallRunningTag))
	{
		WallJump();
		//대각선 반대방향으로 점프
		return;
	}


	if (TryParkourJump()) //[Vault, swing]
	{
		return;
	}
		

	Player->Jump();
}


void UParkourComponent::Input_Up_End(const FInputActionValue& Value)
{
	// if-else로 상황에 따라 다르게 stop()
	Player->StopJumping();
}


void UParkourComponent::Input_Down(const FInputActionValue& Value)
{
	if (TagContainer->HasTag(HangTag)) // 매달린 상태
	{
		Drop();

		return;
	}
	else if(TagContainer->HasTag(FallingTag)) // 낙하상태
	{
		Roll();

		return;
	}
	
	if (!PlayerMovement->IsFalling())
	{
		float CurrentSpeed = Player->GetVelocity().Size2D();
		float RunThreshold = 100.f; //달리는 상태 기준

		if (CurrentSpeed > RunThreshold) // 달리는 상태
		{
			StartSliding(); 
		}
		else // 멈춤/걷기
		{
			if (TagContainer->HasTag(CrouchedTag))
			{
				UnCrouch(); 
			}
			else
			{
				Crouch(); 
			}
		}
	}
	
}


void UParkourComponent::Sprint(const FInputActionValue& Value)
{
	if (PlayerMovement->IsFalling())
	{
		StartAirDash();
	}
	else
	{
		StartDash();
	}
}




void UParkourComponent::Interaction()
{
	UE_LOG(LogTemp, Warning, TEXT("Interaction"));
	// 해당 액터 Interface 작성해서 공통적으로 함수가지게 하고
	// 각 액터마다 다르게 로직 실행되도록 구현
}

//-----------------------------------


/* [Action Logic & called Animation Motion Warping] */

/* Up Key Actions */
void UParkourComponent::ShimmyJump()
{
}
void UParkourComponent::RopeJump()
{
}
void UParkourComponent::Mantle()
{
}
void UParkourComponent::WallJump()
{
}
bool UParkourComponent::TryParkourJump()
{
	return false;
}
/*---*/

/* Down Key Actions */
void UParkourComponent::StartSliding()
{
	UE_LOG(LogTemp, Warning, TEXT("Sliding Start!!"));


	/* End Sliding */
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UParkourComponent::EndSliding, 0.5f);

	TagContainer->AddTag(SlidingTag);
}

void UParkourComponent::EndSliding()
{
	UE_LOG(LogTemp, Warning, TEXT("Sliding End!!"));

	/* 슬라이딩 도중 위에 막힌채로 멈추면 crouch로 변경 로직 */
	TagContainer->RemoveTag(SlidingTag);
}

void UParkourComponent::Crouch()
{
	UE_LOG(LogTemp, Warning, TEXT("Crouch"));

	TagContainer->AddTag(CrouchedTag);
}

void UParkourComponent::UnCrouch()
{
	UE_LOG(LogTemp, Warning, TEXT("Stand Up"));

	TagContainer->RemoveTag(CrouchedTag);
}

void UParkourComponent::Drop()
{
	UE_LOG(LogTemp, Warning, TEXT("Drop"));

	TagContainer->RemoveTag(HangTag);

	// 매달려있는 상태에서 손 놓기
	// 봉을 타고 있지 않다면 falling 상태
	// 
	// 
	// 봉이라면 봉잡고 쭉 내려가기
	// 봉을 타고 있는 상태에서는 falling x
}

void UParkourComponent::Roll()
{
	UE_LOG(LogTemp, Warning, TEXT("Roll"));

	TagContainer->RemoveTag(FallingTag);

	// 추락 상태에서 땅에 가까울때 호출되면 앞구르기로 안전하게 착지
}
/*---*/

/* Sprint Key Actions */
void UParkourComponent::StartDash()
{
	UE_LOG(LogTemp, Warning, TEXT("Dash!!"));

	const FVector ForwardDir = Player->GetActorRotation().Vector();
	Player->LaunchCharacter(ForwardDir * DashDistance, true, true);

	/* play the dash montage
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		const float MontageLength = AnimInstance->Montage_Play(DashMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);

		// has the montage played successfully?
		if (MontageLength > 0.0f)
		{
			AnimInstance->Montage_SetEndDelegate(OnDashMontageEnded, DashMontage);
		}
	}*/

	TagContainer->AddTag(DashTag);

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UParkourComponent::EndDash, 1.0f);
}

void UParkourComponent::EndDash()
{
	UE_LOG(LogTemp, Warning, TEXT("Dash End"));

	TagContainer->RemoveTag(DashTag);
}

void UParkourComponent::StartAirDash()
{
	UE_LOG(LogTemp, Warning, TEXT("Air Dash!!"));

	const FVector ForwardDir = Player->GetActorRotation().Vector();
	Player->LaunchCharacter(ForwardDir * DashDistance, true, true);

	/* play the dash montage
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		const float MontageLength = AnimInstance->Montage_Play(DashMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);

		// has the montage played successfully?
		if (MontageLength > 0.0f)
		{
			AnimInstance->Montage_SetEndDelegate(OnDashMontageEnded, DashMontage);
		}
	}*/

	TagContainer->AddTag(AirDashTag);

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UParkourComponent::EndAirDash, 1.0f);
}

void UParkourComponent::EndAirDash()
{
	UE_LOG(LogTemp, Warning, TEXT("AirDash End"));

	TagContainer->RemoveTag(AirDashTag);
}

/*---*/

/// <summary>
/// Move Function
/// </summary>
void UParkourComponent::DoJump()
{
	
	int LastHitIndex = 0;
	FVector HitLocation;
	FHitResult HitResult;

	/* Create a Three Trace(Low, Middle, High) to Check the Block Height */
	for (int i = 0; i < TraceCount_BlockHeight; i++)
	{

		FVector StartLocation = Player->GetActorLocation() + FVector(0.f, 0.f, TraceGap_BlockHeight * i);
		FVector EndLocation = StartLocation + Player->GetActorForwardVector() * TraceLength_BlockHeight;

		// test 후 channel change
		bool bHitBlock = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECollisionChannel::ECC_GameTraceChannel1);
		DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 2.f, 0, 1.f);

		if (bHitBlock)
		{
			LastHitIndex = i;
			HitLocation = HitResult.Location;
		}
	}

	if (LastHitIndex == TraceCount_BlockHeight - 1) // Hang or Jump
	{
		DoHighJump(HitResult);
	}
	else if (0 < LastHitIndex && LastHitIndex < TraceCount_BlockHeight - 1) // Vault or Mantle
	{
		DoMiddleJump(HitLocation);
	}
	else
	{
		Player->Jump();
	}
}

void UParkourComponent::DoMiddleJump(const FVector& HitLocation)
{
	int LastHitIndex = 0;
	FHitResult VerticalHitResult;

	float CapsuleRadius = 5.0f;
	float CapsuleHalfHeight = 10.0;

	/* Create a Traces to Check the Block Vertical */
	for (int i = 0; i < TraceCount_BlockVertical; i++)
	{
		FHitResult HitResult;
		
		FVector StartLocation = HitLocation + FVector(0.f, 0.f, 50.f) + (Player->GetActorForwardVector() * (TraceGap_BlockVertical * i));
		FVector EndLocation = StartLocation + FVector(0.f, 0.f, -50.f);

		// test 후 channel change
		bool bHitBlock = GetWorld()->SweepSingleByChannel(HitResult, StartLocation, EndLocation, FQuat::Identity, ECollisionChannel::ECC_GameTraceChannel1, FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight));
		DrawDebugCapsule(GetWorld(), HitResult.Location, CapsuleHalfHeight, CapsuleRadius, FQuat::Identity, FColor::Blue, false, 2.f, 0, 1.f);
		
		if (HitResult.bStartPenetrating) // The trace started in penetration
		{
			UE_LOG(LogTemp, Warning, TEXT("The trace started in penetration"));

			break;
		}
		else if (bHitBlock)
		{
			LastHitIndex = i;
			VerticalHitResult = HitResult;

			if (i == 0)
			{
				VaultStartPos = VerticalHitResult.ImpactPoint;
			}
			
			VaultMiddlePos = VerticalHitResult.ImpactPoint;
			bCanWarp = true;

			DrawDebugCapsule(GetWorld(), VaultMiddlePos, 5.0f, 5.0f, FQuat::Identity, FColor::Yellow, false, 2.f, 0, 1.f);
		}
		else
		{
			break;
		}
	}

	/* 뒤에 공간 상태에 따라 Mantle or Vault 설정 */
	if (LastHitIndex == TraceCount_BlockVertical - 1) /* 뒤에 넘어갈 공간이 없는 경우 */
	{
		UE_LOG(LogTemp, Warning, TEXT("not enough space"));
		DoMantle();
	}
	else /* 뒤에 넘어갈 공간이 있는 경우 뒷공간 특징 확인 */
	{
		FHitResult LandHitResult;
		FVector StartLocation = VerticalHitResult.Location + (Player->GetActorForwardVector() * TraceGap_BlockVertical);
		FVector EndLocation = StartLocation + FVector(0.f, 0.f, -300.f);

		bool bHitBlock = GetWorld()->LineTraceSingleByChannel(LandHitResult, StartLocation, EndLocation, ECollisionChannel::ECC_GameTraceChannel1);
		DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Yellow, false, 2.f, 0, 1.f);

		if (bHitBlock) /* 뒤에 공간이 바닥인 경우 */
		{
			float LandZ = LandHitResult.Location.Z;
			float PlayerFeetZ = Player->GetMesh()->GetComponentLocation().Z;

			VaultLandPos = LandHitResult.Location;

			/* vault 전후 바닥 높이 차이 확인 */
			if (FMath::IsWithinInclusive(LandZ, PlayerFeetZ - 100.f, PlayerFeetZ + 50.f))
			{ 
				DoVault(); // 차이 +50 -100 사이인 경우
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("lans z isRange out !!!"));
				DoMantle(); // 차이 +50 -100 초과인 경우  
			}
		}
		else /* 뒤에 공간이 낭떨어지인 경우 */
		{
			UE_LOG(LogTemp, Warning, TEXT("space is cliff"));
			DoMantle();
		}
	}
}


/* Hang or just Jump */
void UParkourComponent::DoHighJump(const FHitResult& HitResult)
{
	FHitResult SkyHitResult;
	FVector StartLocation = HitResult.TraceStart + FVector(0.f, 0.f, TraceGap_BlockHeight);
	FVector EndLocation = StartLocation + Player->GetActorForwardVector() * TraceLength_BlockHeight;

	bool bHitBlock = GetWorld()->LineTraceSingleByChannel(SkyHitResult, StartLocation, EndLocation, ECollisionChannel::ECC_GameTraceChannel1);
	DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Yellow, false, 2.f, 0, 1.f);

	if (SkyHitResult.bStartPenetrating) /* bHitResult is false but blocking*/
	{
		Player->Jump();
	}
	else if (!bHitBlock)
	{
		DoHang();
	}
	else
	{
		Player->Jump();
	}
}


void UParkourComponent::DoVault()
{
	UE_LOG(LogTemp, Warning, TEXT("Vaulting !!!"));

	OnVaultMotionWarping.Broadcast();
}


void UParkourComponent::DoMantle()
{
	UE_LOG(LogTemp, Warning, TEXT("Mantle !!!"));
}


void UParkourComponent::DoHang()
{
	UE_LOG(LogTemp, Warning, TEXT("Hang !!!"));
}

/// <summary>
/// //////////////////////////////////
/// </summary>


bool UParkourComponent::CanSliding()
{
	int LastHitIndex = 0;
	FHitResult HitResult;

	/* Check the Middle & Low traces */
	for (int i = 0; i < 3; i++)
	{
		FVector StartLocation = Player->GetActorLocation() + FVector(0.f, 0.f, TraceGap_BlockHeight * i);
		FVector EndLocation = StartLocation + Player->GetActorForwardVector() * TraceLength_BlockHeight;

		// test 후 channel change
		bool bHitBlock = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECollisionChannel::ECC_GameTraceChannel1);
		DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 2.f, 0, 1.f);

		if (bHitBlock)
		{
			LastHitIndex = i;
			break;
		}
	}

	if (LastHitIndex == 2)
	{
		return true;
	}

	return false;
}

bool UParkourComponent::CheckHitWall()
{
	/*if (!Player || !PlayerMovement) return;

	if (PlayerMovement->IsFalling())
	{
		FHitResult HitResult;
		FVector StartLocation = Player->GetActorLocation();
		FVector EndLocation = StartLocation + Player->GetActorForwardVector() * 100.f;

		// "ECollisionChannel::ECC_GameTraceChannel1" is Custom "Jump Wall" Channel
		bool bHitWall = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECollisionChannel::ECC_GameTraceChannel1);

		if (bHitWall) // 벽 점프 상태 활성화
		{

			bIsWallJump = true;

			SlowJumpToLand();

			DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 2.f, 0, 1.f);
		}
		else
		{
			bIsWallJump = false;
		}
	}
	else // 땅에 닿으면 벽 점프 상태 해제
	{
		bIsWallJump = false;
	}
	*/
	return false;
}

bool UParkourComponent::IsWallJump()
{
	//UE_LOG(LogTemp, Warning, TEXT("Wall Jump %d"), bIsWallJump);
	//return bIsWallJump;
	return false;
}

void UParkourComponent::DoWallJump()
{
	/*
	UE_LOG(LogTemp, Warning, TEXT("Is Wall Jumping"));
	
	const FVector ForwardDir = Player->GetActorForwardVector();
	FVector NewDir = ForwardDir * 1000.f + (0.f, 0.f, 1000.f);
	Player->LaunchCharacter(NewDir, true, true);

	FRotator NewRotation = Player->GetActorRotation();
	NewRotation.Yaw += 180.f;
	Player->SetActorRotation(NewRotation);
	*/
}

void UParkourComponent::SlowJumpToLand()
{
	//UE_LOG(LogTemp, Warning, TEXT("SlowJumpToLand"));
	/*
	float WallSlideSpeed = -300.f;

	FVector CurrentVelocity = PlayerMovement->Velocity;
	
	CurrentVelocity.Z = FMath::Max(CurrentVelocity.Z, WallSlideSpeed);
	PlayerMovement->Velocity = CurrentVelocity;
	*/
}







