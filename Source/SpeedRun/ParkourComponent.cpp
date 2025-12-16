// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourComponent.h"
#include "SpeedRunCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Animation/AnimMontage.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "MotionWarpingComponent.h"

// Sets default values for this component's properties
UParkourComponent::UParkourComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	bCanWarp = false;

	

}


// Called when the game starts
void UParkourComponent::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<ASpeedRunCharacter>(GetOwner());

	if (Player)
	{
		Movement = Player->GetCharacterMovement();
	
		if (Movement)
		{
			// 1. 앉기 활성화
			Movement->GetNavAgentPropertiesRef().bCanCrouch = true;

			DefaultCrouchedHalfHeight = Movement->CrouchedHalfHeight;

			// 2. 앉았을 때 캡슐 높이, 속도 설정
			Movement->MaxWalkSpeedCrouched = MaxCrouchSpeed;
			Movement->CrouchedHalfHeight = CrouchedHalfHeight;
		}
		
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

void UParkourComponent::AddTag(FGameplayTag NewTag)
{
	ParkourTags.AddTag(NewTag);
}

void UParkourComponent::RemoveTag(FGameplayTag Tag)
{
	ParkourTags.RemoveTag(Tag);
}

bool UParkourComponent::HasTag(FGameplayTag Tag)
{
	return ParkourTags.HasTag(Tag);
}

/* [Called when the Player Action Changes for input key ] */

void UParkourComponent::Input_Up_Start(const FInputActionValue& Value)
{
	if (HasTag(HangTag)) // 매달린 상태
	{
		if (HasTag(ShimmyTag))
		{
			ShimmyJump();
		}
		else if (HasTag(RopeTag))
		{
			RopeJump();
		}
		else
		{
			Mantle();
		}
		return;
	}
	

	if (HasTag(WallRunningTag))
	{
		WallJump();
		//대각선 반대방향으로 점프
		return;
	}


	TryParkourJump();
		
}


void UParkourComponent::Input_Up_End(const FInputActionValue& Value)
{
	// if-else로 상황에 따라 다르게 stop()
	Player->StopJumping();
}


void UParkourComponent::Input_Down(const FInputActionValue& Value)
{
	if (Movement->IsFalling())
	{
		if (HasTag(HangTag)) // 매달린 상태
		{
			Drop();
		}
		else if (HasTag(FallingTag)) // 낙하상태
		{
			Roll();
		}
		return;
	}
	else
	{
		float CurrentSpeed = Player->GetVelocity().Size2D();
		float RunThreshold = 150.f; //달리는 상태 기준

		if (CurrentSpeed > RunThreshold) // 달리는 상태
		{
			if (!HasTag(SlidingTag))
			{
				StartSliding();
			}
		}
		else // 멈춤/걷기
		{
			if (HasTag(CrouchedTag))
			{
				UnCrouch(); 
			}
			else
			{
				Crouch(); 
			}
		}
		return;
	}
	
}


void UParkourComponent::Sprint(const FInputActionValue& Value)
{
	if (Movement->IsFalling())
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
	if (!bIsOnLedge)
	{
		float InitialZOffset = Movement->IsFalling() ? 50.f : 25.f;
		float TraceVertical = Movement->IsFalling() ? 75.f : 10.f;
		
		TraceLedge(InitialZOffset, 100.f, TraceVertical);

		if (!bLedgeDetected)
		{
			Player->Jump();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Hang On Ledge"));
			HangOnLedge();
		}
	}
	
	return false;
}
/*---*/

/* Down Key Actions */
void UParkourComponent::StartSliding()
{
	UE_LOG(LogTemp, Warning, TEXT("Sliding Start!!"));

	AddTag(SlidingTag);

	Player->Crouch(); //기본 Crouch

	FVector StartLocation = Player->GetActorLocation();
	FVector ForwardVector = Player->GetActorForwardVector();

	FVector TargetLocation = StartLocation + (ForwardVector * SlideDistance);
	FRotator TargetRotation = Player->GetActorRotation();

	FParkourActionPayload ParkourActionPayload = {
		.AnimMontage = SlideAnim,
		.TargetName = SlideTargetName,  
		.TargetLocation = TargetLocation, 
		.TargetRotation = TargetRotation,
	};

	ParkourActionPayload.OnParkourAnimEndedDelegate.BindDynamic(this, &UParkourComponent::EndSliding);

	Player->PlayMotionWarping(ParkourActionPayload);

}
	

void UParkourComponent::EndSliding()
{
	UE_LOG(LogTemp, Warning, TEXT("Sliding End!!"));

	RemoveTag(SlidingTag);

	Player->UnCrouch(); //기본 UnCrouch

	/* 슬라이딩 도중 위에 막힌채로 멈추면 crouch로 변경 로직 */
 
}

void UParkourComponent::Crouch()
{
	UE_LOG(LogTemp, Warning, TEXT("Crouch"));

	AddTag(CrouchedTag);

	Player->Crouch(); //기본 Crouch

	FParkourActionPayload ParkourActionPayload;
	ParkourActionPayload.AnimMontage = CrouchAnim;
	Player->PlayMotionWarping(ParkourActionPayload);
}

void UParkourComponent::UnCrouch()
{
	UE_LOG(LogTemp, Warning, TEXT("Stand Up"));

	RemoveTag(CrouchedTag);

	Player->UnCrouch(); //기본 UnCrouch

//	Movement->CrouchedHalfHeight = DefaultCrouchedHalfHeight;
	//Movement->GetNavAgentPropertiesRef().bCanCrouch = false;

	//Player->PlayMotionWarping(ParkourActionPayload);
}

void UParkourComponent::Drop()
{
	UE_LOG(LogTemp, Warning, TEXT("Drop"));

	RemoveTag(HangTag);

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

	RemoveTag(FallingTag);

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

	AddTag(DashTag);

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UParkourComponent::EndDash, 0.5f);
}

void UParkourComponent::EndDash()
{
	UE_LOG(LogTemp, Warning, TEXT("Dash End"));

	RemoveTag(DashTag);
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

	AddTag(AirDashTag);

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UParkourComponent::EndAirDash, 0.5f);
}

void UParkourComponent::EndAirDash()
{
	UE_LOG(LogTemp, Warning, TEXT("AirDash End"));

	RemoveTag(AirDashTag);
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

}


void UParkourComponent::DoMantle()
{
	UE_LOG(LogTemp, Warning, TEXT("Mantle !!!"));
}


void UParkourComponent::DoHang()
{
	UE_LOG(LogTemp, Warning, TEXT("Hang !!!"));
}

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

/*
* Hang & Climb Up
*/

void UParkourComponent::TraceLedge(float InitialZOffset, float TraceDistance, float TraceVertical)
{
	
	// Check for Ledge.(Horizontal)
	for (int i = 0; i < 10; i++)
	{
		FHitResult HitResult_H;
		float ZOffset = InitialZOffset + (i * TraceVertical);
		FVector StartLocation = Player->GetActorLocation() + FVector(0.f, 0.f, ZOffset);
		FVector EndLocation = StartLocation + (Player->GetActorForwardVector() * TraceDistance);

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Player);
		float SphereRadius = 10.f;
		bool bHorizontalHit = GetWorld()->SweepSingleByChannel(HitResult_H, StartLocation, EndLocation, FQuat::Identity, ECollisionChannel::ECC_GameTraceChannel1, FCollisionShape::MakeSphere(SphereRadius), Params);

		DrawDebugSphere(GetWorld(), HitResult_H.Location, SphereRadius, 12, FColor::Blue, false, 2.0f);

		// Check for Surface. (Vertical)
		if (bHorizontalHit)
		{
			FHitResult HitResult_V;
			bool bVerticalHit = GetWorld()->SweepSingleByChannel(HitResult_V, HitResult_H.ImpactPoint + FVector(0.f, 0.f, 20.f), HitResult_H.ImpactPoint, FQuat::Identity, ECollisionChannel::ECC_GameTraceChannel1, FCollisionShape::MakeSphere(SphereRadius), Params);

			DrawDebugSphere(GetWorld(), HitResult_V.Location, SphereRadius, 12, FColor::Yellow, false, 2.0f);

			// if Distance is 0, the trace started inside an wall (InitialOverlap).
			if (bVerticalHit && HitResult_V.Distance > 0)
			{
				bLedgeDetected = true;
				LedgeLocation = HitResult_V.ImpactPoint;
				LedgeNormal = HitResult_H.ImpactNormal;

				return;
			}
		}
	}

	bLedgeDetected = false;
	LedgeLocation = FVector(0.f,0.f,0.f);
	LedgeNormal = FVector(0.f, 0.f, 0.f);

	return;
}

void UParkourComponent::HangOnLedge()
{
	FTimerHandle TimerHandle;

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UParkourComponent::CheckIfBelowLedgeHasSurface, 0.01f, false);

	UCapsuleComponent* CapsuleComponent = Player->GetCapsuleComponent();
	float CapsuleHalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();

	// 1. 플레이어가 벽에 붙어 있을 z 위치 (팔로 매달리고 있어야 하므로 LedgeLocation보다 조금 아래로 내려야한다.)
	FVector HeightLocation = LedgeLocation - (0.f, 0.f, CapsuleHalfHeight + 100.f);

	// 2. 벽 표면의 법선 벡터를 바탕으로 회전벡터 생성
	FRotator LedgeRotation = UKismetMathLibrary::MakeRotFromX(LedgeNormal);

	// 3. 벽과 플레이어사이 거리 설정 : 몸은 벽과 조금 떨어져 있다.(48만큼 떨어뜨림)
	FVector AwayLocation = UKismetMathLibrary::GetForwardVector(LedgeRotation) * 48.f;

	// 4. 위치 계산
	FVector HangLocation = HeightLocation + AwayLocation;

	// 5. 회전 계산 : 플레이어는 벽의 법선벡터와 반대방향으로 바라봐야 한다.
	FRotator HangRotation = LedgeRotation + FRotator(0.f, -180.f, 0.f);

	
	Player->SetActorRotation(HangRotation);

	if (!Movement->IsFalling())
	{
		Movement->SetMovementMode(EMovementMode::MOVE_Flying);
		if (!bIsOnLedge)
		{
			UMotionWarpingComponent* WarpComponent = Player->GetMotionWarpingComponent();

			WarpComponent->AddOrUpdateWarpTargetFromLocationAndRotation("LedgePosition", HangLocation, HangRotation);

			UAnimInstance* AnimInstance = Player->GetMesh()->GetAnimInstance();

			if (bBelowLedgeHasSurfaceL)
			{
				AnimInstance->Montage_Play(IdleToBracedHang);
			}
			else
			{
				AnimInstance->Montage_Play(IdleToFreeHang);
			}

			//bIsOnLedge = true;

			//Movement->StopMovementImmediately();
		}
		else
		{
			SetFlying(HangLocation,HangRotation);
		}
	}
	else
	{
		SetFlying(HangLocation, HangRotation);
	}
}


void UParkourComponent::CheckIfBelowLedgeHasSurface()
{
	USkeletalMeshComponent* Mesh = Player->GetMesh();

	FHitResult HitResultL;
	FVector StartLocationL = Mesh->GetComponentLocation() + FVector(25.f, 0.f, 70.f);
	FVector EndLocationL = StartLocationL + (Player->GetActorForwardVector() * 80.f);
	bBelowLedgeHasSurfaceL = GetWorld()->LineTraceSingleByChannel(HitResultL, StartLocationL, EndLocationL, ECollisionChannel::ECC_GameTraceChannel1);
	DrawDebugLine(GetWorld(), StartLocationL, EndLocationL, FColor::Red, false, 2.f, 0, 1.f);

	FHitResult HitResultR;
	FVector StartLocationR = Mesh->GetComponentLocation() + FVector(-25.f, 0.f, 70.f);
	FVector EndLocationR = StartLocationR + (Player->GetActorForwardVector() * 80.f);
	bBelowLedgeHasSurfaceR = GetWorld()->LineTraceSingleByChannel(HitResultR, StartLocationL, EndLocationL, ECollisionChannel::ECC_GameTraceChannel1);
	DrawDebugLine(GetWorld(), StartLocationR, EndLocationR, FColor::Red, false, 2.f, 0, 1.f);

	UE_LOG(LogTemp, Warning, TEXT("Right Foot Surface is %f"), (float)bBelowLedgeHasSurfaceR);
}

void UParkourComponent::SetFlying(FVector HangLocation, FRotator HangRotation)
{
	UCapsuleComponent* CapsuleComponent = Player->GetCapsuleComponent();

	FVector TargetLocation = HangLocation + (0.f, 0.f, CapsuleComponent->GetScaledCapsuleHalfHeight());

	Player->SetActorLocationAndRotation(TargetLocation, HangRotation);

	Movement->SetMovementMode(EMovementMode::MOVE_Flying);

	bIsOnLedge = true;

	Movement->StopMovementImmediately();
}

void UParkourComponent::SetIsOnLedge(bool Value)
{
	bIsOnLedge = Value;
}

bool UParkourComponent::GetIsOnLedge()
{
	return bIsOnLedge;
}

bool UParkourComponent::GetBelowLedgeHasSurfaceR()
{
	return bBelowLedgeHasSurfaceR;
}

bool UParkourComponent::GetBelowLedgeHasSurfaceL()
{
	return bBelowLedgeHasSurfaceL;
}
