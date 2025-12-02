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

	bIsDashing = false;
	bIsSliding = false;
	bIsCrouch = false;
	bIsDrop = false;
	bIsVaulting = false;
	bIsMantling = false;
	bIsHanging = false;
}


// Called when the game starts
void UParkourComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<ASpeedRunCharacter>(GetOwner());

	if (Player)
	{
		PlayerMovement = Player->GetCharacterMovement();
	}
}


// Called every frame
void UParkourComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}
//-----------------------

/* [Called when the Player State Changes for input key ] */
void UParkourComponent::DashOrSlide()
{
	if (PlayerMovement->IsFalling())
	{
		UE_LOG(LogTemp, Warning, TEXT("IsFalling!!"));

		return;
	}

	if (bIsDashing || bIsSliding) 
	{
		UE_LOG(LogTemp, Warning, TEXT("already dashing or sliding!!"));

		return;
	}

	if (CanSliding())
	{
		StartSliding();
	}
	else
	{
		StartDash();
	}
}


void UParkourComponent::CrouchOrDrop()
{
	if (bIsDrop)
	{
		UE_LOG(LogTemp, Warning, TEXT("already drop!!"));

		return;
	}


	if (PlayerMovement->IsFalling())
	{
		DoDrop();
	}
	else
	{
		DoCrouch();
	}

	return;
}


void UParkourComponent::ParkourJump()
{
	if (CanJumping())
	{
		DoJump();
	}
	
	return;
}
//-----------------------------------

void UParkourComponent::StartDash()
{
	UE_LOG(LogTemp, Warning, TEXT("Dash!!"));

	bIsDashing = true;

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

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UParkourComponent::EndDash, 1.0f);

}


void UParkourComponent::EndDash()
{
	if (bIsDashing)
	{
		UE_LOG(LogTemp, Warning, TEXT("Dash Reset!!"));

		bIsDashing = false;
	}
}


void UParkourComponent::StartSliding()
{
	UE_LOG(LogTemp, Warning, TEXT("Sliding!!"));

	/* Start Sliding */
	bIsSliding = true;

	/* End Sliding */
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UParkourComponent::EndSliding, 0.5f);
}


void UParkourComponent::EndSliding()
{
	if (bIsSliding)
	{
		UE_LOG(LogTemp, Warning, TEXT("Sliding Reset!!"));

		bIsSliding = false;

		/* 슬라이딩 도중 위에 막힌채로 멈추면 crouch로 변경 로직 */
	}
}

void UParkourComponent::DoCrouch()
{
	if (bIsCrouch)
	{
		// 위에 장애물이 없을 시 (일어설수있는 공간이 있는 경우)
		// crouch 비활성화 -> Idle로 변경

		UE_LOG(LogTemp, Warning, TEXT("Stand Up"));

		bIsCrouch = false;
	}
	else
	{
		// crouch 활성화
		UE_LOG(LogTemp, Warning, TEXT("Crouch"));

		bIsCrouch = true;

	}
}

void UParkourComponent::DoDrop()
{

	UE_LOG(LogTemp, Warning, TEXT("Drop"));

	// 매달려있는 상태에서 손 놓기
	//  벽 두개 사이 작은틈이 있으면 벽타고 마찰로 내려가기
	// 봉이라면 봉잡고 쭉 내려가기
	
	// 손 놓기 아래로 떨어지기
	// bIsDrop = true;
	
	/* 바닥에 착지했으면 bisdrop = false; */

}
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

bool UParkourComponent::CanJumping()
{
	if (PlayerMovement->IsFalling() || bIsVaulting || bIsMantling) return false;

	return true;
}

bool UParkourComponent::CanVaulting()
{
	/* Create a Three Trace(Low, Middle, High) to Check the Block Height */
	int LastHitIndex = 0;
	FVector HitLocation;
	FHitResult HitResult;

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
			HitLocation = HitResult.Location;
		}
	}

	return false;
}

/// <summary>
/// ////////////////////////////////////////////////
/// </summary>
/// <returns></returns>

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







