// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/USPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/USCameraData.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UMG/Public/Blueprint/UserWidget.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "../MiniView/MiniViewComponent.h"
#include "Components/CapsuleComponent.h"

AUSPlayer::AUSPlayer()
{
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->bUsePawnControlRotation = false;
	bUseControllerRotationYaw = true;

	static ConstructorHelpers::FClassFinder<UUserWidget> HUDWidgetRef(TEXT("/Game/Study/Character/UI/WBP_HUD.WBP_HUD_C"));
	if (HUDWidgetRef.Succeeded())
	{
		HUDWidgetClass = HUDWidgetRef.Class;
	}

	for (uint32 ViewType = (uint8)EViewType::None + 1; ViewType < (uint8)EViewType::Max; ++ViewType)
	{
		// �̸��� ViewType ���� ���Խ��� �� SceneCapture ������Ʈ�� ������ �̸��� �����մϴ�.
		FName ComponentName = *FString::Printf(TEXT("MiniViewSceneCapture_%d"), ViewType);
		SceneCapture.Add(static_cast<EViewType>(ViewType), CreateDefaultSubobject<USceneCaptureComponent2D>(ComponentName));
	}
}

void AUSPlayer::BeginPlay()
{
	Super::BeginPlay();

	SetupCemeraSprigArm();
	CameraChange();
	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
		HUDWidget->AddToViewport();
	}
}

// �� ������ �ƴ϶� ���ð��� �� �������� ���������� ����� ��ü�ϴ� �������� �ٲ����ҵ�
void AUSPlayer::SetViewData(const UUSCameraData* CameraData)
{
	if (CameraData == nullptr)
		return;

	bUseControllerRotationYaw = CameraData->bUseControllerRotationYaw;
	if (bUseControllerRotationYaw == false)
	{
		Controller->SetControlRotation(FRotator::ZeroRotator);
	}
	
	SetInputContextChange(CameraData->InputMappingContext);

	
}

void AUSPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	if (InputActionMap.Contains(EInputKey::Move))
	{
		EnhancedInputComponent->BindAction(InputActionMap[EInputKey::Move], ETriggerEvent::Triggered, this, &ThisClass::Move);
	}

	if (InputActionMap.Contains(EInputKey::Look))
	{
		EnhancedInputComponent->BindAction(InputActionMap[EInputKey::Look], ETriggerEvent::Triggered, this, &ThisClass::Look);
	}

	if (InputActionMap.Contains(EInputKey::CameraChange))
	{
		EnhancedInputComponent->BindAction(InputActionMap[EInputKey::CameraChange], ETriggerEvent::Triggered, this, &ThisClass::CameraChange);
	}

	if (InputActionMap.Contains(EInputKey::ClickMove))
	{
		EnhancedInputComponent->BindAction(InputActionMap[EInputKey::ClickMove], ETriggerEvent::Triggered, this, &ThisClass::ClickMove);
		EnhancedInputComponent->BindAction(InputActionMap[EInputKey::ClickMove], ETriggerEvent::Completed, this, &ThisClass::ClickMove);
	}

	if (InputActionMap.Contains(EInputKey::Jump))
	{
		EnhancedInputComponent->BindAction(InputActionMap[EInputKey::Jump], ETriggerEvent::Triggered, this, &ThisClass::Jump);
		EnhancedInputComponent->BindAction(InputActionMap[EInputKey::Jump], ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}

}

void AUSPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (bIsClimbingUp)
		return;
	// �̰Ը´°�??
	FVector StartPoint = GetCapsuleComponent()->GetComponentLocation();

	FVector ForwardVector = GetCapsuleComponent()->GetForwardVector() * 45;
	FVector EndPoint = StartPoint + ForwardVector;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.AddIgnoredActor(this); // �� ���ʹ� Ʈ���̽����� ����
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartPoint,
		EndPoint,
		ECC_Visibility,
		QueryParams
	);

	// ���� Ʈ���̽� ��θ� ����׿����� �׸���
	DrawDebugLine(
		GetWorld(),
		StartPoint,
		EndPoint,
		FColor::Red,
		false,  // ���������� �׸� ������ ����
		1.0f,   // ���� �ð�
		0,      // DepthPriority
		1.0f    // ���� �β�
	);

	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString::Printf(TEXT("Hit: %s"), bHit ? *FString("true") : *FString("false")));

	if (bHit)
	{
		// �Ӹ� �κ��� ���� �������� �ִ��� Ȯ��
		FHitResult HitResult2;
		FVector HeadPoint = StartPoint + FVector(0, 0, 110) + ForwardVector;
		bIsClimbingEdge = !GetWorld()->LineTraceSingleByChannel(
			HitResult2,
			StartPoint,
			HeadPoint,
			ECC_Visibility,
			QueryParams
		);

		if (bIsClimbingEdge)
		{
			bIsClimbingUp = true;
			// �ö󰡴� ��Ÿ�� ����
			// ��Ÿ�� �����߿� ���� ��Ÿ���� �ʿ�


			// �ö󰡴� �ൿ ����
			bIsClimbing = false;
			UCharacterMovementComponent* CharMoveComp = Cast<UCharacterMovementComponent>(GetMovementComponent());
			CharMoveComp->SetMovementMode(EMovementMode::MOVE_Walking);
			CharMoveComp->bOrientRotationToMovement = true;
		}
		DrawDebugLine(
			GetWorld(),
			StartPoint,
			HeadPoint,
			FColor::Blue,
			false,  // ���������� �׸� ������ ����
			1.0f,   // ���� �ð�
			0,      // DepthPriority
			1.0f    // ���� �β�
		);


		FVector Normal = HitResult.Normal; // �浹 ������ ���� ����
		FRotator Rotation = FRotationMatrix::MakeFromX(Normal).Rotator();

		double NewYawValue = Rotation.Yaw + 180;
		FRotator CurrentRotation = GetActorRotation();
		FRotator NewRotation = FRotator(CurrentRotation.Pitch, NewYawValue, CurrentRotation.Roll);

		SetActorRotation(NewRotation);

		// ���⼭ ���� �ѹ���
		if (bIsClimbing == false)
		{
			bIsClimbing = true;

			GetMovementComponent()->Velocity = FVector(0, 0, 0);

			UCharacterMovementComponent* CharMoveComp = Cast<UCharacterMovementComponent>(GetMovementComponent());
			CharMoveComp->SetMovementMode(EMovementMode::MOVE_Flying);
			CharMoveComp->bOrientRotationToMovement = false;
		}
		
	}
	else
	{
		if (bIsClimbing)
		{
			bIsClimbing = false;
			UCharacterMovementComponent* CharMoveComp = Cast<UCharacterMovementComponent>(GetMovementComponent());
			CharMoveComp->SetMovementMode(EMovementMode::MOVE_Walking);
			CharMoveComp->bOrientRotationToMovement = true;;
		}	
	}
}

void AUSPlayer::SetInputContextChange(class UInputMappingContext* InputMappingContext)
{
	APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
		if (InputMappingContext == nullptr)
			return;

		UInputMappingContext* NewMappingContext = InputMappingContext;
		if (NewMappingContext)
		{
			Subsystem->AddMappingContext(NewMappingContext, 0);
		}
	}
}

void AUSPlayer::Move(const FInputActionValue& Value)
{
	ClickInputClear();

	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (bIsClimbing)
	{
		if (bIsClimbingEdge == false)
		{
			AddMovementInput(GetActorUpVector(), MovementVector.X);
			AddMovementInput(GetActorRightVector(), MovementVector.Y);
		}
		else // �̋� �ѹ� ũ�� �÷�����ϴµ�.. �ϴ� ����� �����...
		{
		}
	}
	else
	{
		AddMovementInput(ForwardDirection, MovementVector.X);
		AddMovementInput(RightDirection, MovementVector.Y);
	}

	
}

void AUSPlayer::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);

	//UE_LOG(LogTemp, Warning, TEXT("Look %s"), *LookAxisVector.ToString());
}

void AUSPlayer::CameraChange()
{
	// ���� ������ ���� �������� ������Ʈ
	CurrentViewType = GetNextViewType(CurrentViewType);

	// ���ο� ������ �°� ī�޶� ������ ����
	if (CameraTypeMap[CurrentViewType])
	{
		SetViewData(CameraTypeMap[CurrentViewType]); // �̰� ī�޶� �����̶�� �����ϳ�...�ϴ���
	}
	
	// �������Ͽ� ����
	if (CemeraSprigArm[CurrentViewType])
	{
		FollowCamera->AttachToComponent(CemeraSprigArm[CurrentViewType], FAttachmentTransformRules::KeepRelativeTransform, USpringArmComponent::SocketName);
	}
		
}

void AUSPlayer::ClickMove()
{
	APlayerController* pController = Cast<APlayerController>(GetController());
	if (pController == nullptr)
		return;

	FHitResult Hit;
	bool bHitSuccessful = pController->GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);

	if (bHitSuccessful)
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(pController, Hit.Location);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(pController, nullptr, Hit.Location, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}
}

void AUSPlayer::Jump()
{
	if (bIsClimbing)
	{
		// ��� �ݴ� ����
		FVector ForwardVector = GetCapsuleComponent()->GetForwardVector();
		ForwardVector.Z = 0;
		ForwardVector *= -50;

		LaunchCharacter(ForwardVector, false, false);
		return;
	}
	Super::Jump();
}

void AUSPlayer::ClickInputClear()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		PlayerController->StopMovement();
	}
}


EViewType AUSPlayer::GetNextViewType(EViewType CurrentView)
{
	switch (CurrentView)
	{
	case EViewType::None: return EViewType::FirstPerson;
	case EViewType::FirstPerson: return EViewType::ThirdPerson;
	case EViewType::ThirdPerson: return EViewType::TopDown;
	case EViewType::TopDown: return EViewType::FirstPerson;
	default: return EViewType::None; // �⺻�� ó��
	}
}

void AUSPlayer::SetupCemeraSprigArm()
{
	for (uint8 viewtype = (uint8)EViewType::None; viewtype < (uint8)EViewType::Max; ++viewtype)
	{
		SetCameraSprigArm( static_cast<EViewType>(viewtype));
	}
}

void AUSPlayer::SetCameraSprigArm(EViewType ViewType)
{
	if (CameraTypeMap.Contains(ViewType) == false)
		return;

	const UUSCameraData* CameraData = CameraTypeMap[ViewType];
	if (CameraData == nullptr)
		return;

	//TObjectPtr<USpringArmComponent> SpringArm = CreateDefaultSubobject<USpringArmComponent>(FName(*FString::Printf(TEXT("CameraBoom%d"), static_cast<int32>(ViewType))));
	TObjectPtr<USpringArmComponent> SpringArm = NewObject<USpringArmComponent>(this, FName(*FString::Printf(TEXT("CameraBoom%d"), static_cast<int32>(ViewType))));
	SpringArm->RegisterComponent();

	SpringArm->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	SpringArm->TargetArmLength = CameraData->TargetArmLength;
	SpringArm->SetRelativeRotation(CameraData->RelativeRotation);
	SpringArm->SetRelativeLocation(CameraData->RelativeLocation);
	SpringArm->bUsePawnControlRotation = CameraData->bUsePawnControlRotation;
	SpringArm->bInheritPitch = CameraData->bInheritPitch;
	SpringArm->bInheritYaw = CameraData->bInheritYaw;
	SpringArm->bInheritRoll = CameraData->bInheritRoll;
	SpringArm->bDoCollisionTest = CameraData->bDoCollisionTest;

	CemeraSprigArm.Add(ViewType, SpringArm);

	SceneCapture[ViewType]->AttachToComponent(SpringArm, FAttachmentTransformRules::KeepRelativeTransform);
}
