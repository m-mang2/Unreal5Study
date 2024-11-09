// Fill out your copyright notice in the Description page of Project Settings.


#include "Cropout/Player/USCropoutPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Components/SphereComponent.h"

// Sets default values
AUSCropoutPlayer::AUSCropoutPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 1139.963867f;
	//SpringArm->bUsePawnControlRotation = true;
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bInheritRoll = true;
	SpringArm->SocketOffset = FVector(-300.0, 0, 80.0);
	SpringArm->SetRelativeRotation(FRotator(0.0f, -40.0f, 0.0f)); 


	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	//Camera->bUsePawnControlRotation = false;

	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));

	static ConstructorHelpers::FObjectFinder<UCurveFloat> Curve(TEXT("/Game/Study/Cropout/Data/C_Zoom.C_Zoom"));

	if (Curve.Succeeded())
	{
		ZoomCurve = Curve.Object;
	}

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	//Collision->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	Cursor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cursor"));
}

// Called when the game starts or when spawned
void AUSCropoutPlayer::BeginPlay()
{
	Super::BeginPlay();

	UpdateZoom();
}

// Called every frame
void AUSCropoutPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//MoveTracking();
}

// Called to bind functionality to input
void AUSCropoutPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
		if (BaseInputMappingContext)
		{
			Subsystem->AddMappingContext(BaseInputMappingContext, 0);
		}

		if (VillagerMappingContext)
		{
			Subsystem->AddMappingContext(VillagerMappingContext, 0);
		}
	}

}

void AUSCropoutPlayer::UpdateZoom()
{
	ZoomValue = FMath::Clamp((ZoomDirection * 0.01) + ZoomValue, 0, 1);

	float Alpha = ZoomCurve->GetFloatValue(ZoomValue);

	float TargetArmLength = UKismetMathLibrary::Lerp(800.0f, 40000.0f, Alpha);
	SpringArm->TargetArmLength = TargetArmLength;

	float TargetRotation = UKismetMathLibrary::Lerp(-40.0f, -55.0f, Alpha);
	SpringArm->SetRelativeRotation(FRotator(0.0f, TargetRotation, 0.0f));

	float MaxSpeed = UKismetMathLibrary::Lerp(1000.0f, 6000.0f, Alpha);
	FloatingPawnMovement->MaxSpeed = MaxSpeed;

	Dof();

	float FieldOfView = UKismetMathLibrary::Lerp(20.0f, 15.0f, Alpha);
	Camera->SetFieldOfView(FieldOfView);

}

void AUSCropoutPlayer::Dof()
{
	// ����Ʈ ���μ��� ���� ��ü�� �����ϰ� ���� ����
	FPostProcessSettings PostProcessSettings;
	PostProcessSettings.bOverride_DepthOfFieldFstop = true;    // ������ �� �������̵� ����
	PostProcessSettings.DepthOfFieldFstop = 3.0f;              // ������ �� ���� (f-stop)

	PostProcessSettings.bOverride_DepthOfFieldSensorWidth = true; // ���� �ʺ� �������̵� ����
	PostProcessSettings.DepthOfFieldSensorWidth = 150.0f;         // ���� �ʺ� ���� (mm ����)

	PostProcessSettings.bOverride_DepthOfFieldFocalDistance = true; // ���� �Ÿ� �������̵� ����
	PostProcessSettings.DepthOfFieldFocalDistance = SpringArm->TargetArmLength; // SpringArm ���̸� ���� �Ÿ��� ����

	// ī�޶� ����Ʈ ���μ��� ������ �����մϴ�.
	Camera->PostProcessSettings = PostProcessSettings;
}

void AUSCropoutPlayer::MoveTracking()
{
	FVector PlayerLocation = GetActorLocation();
	FVector NormalizedLocation = PlayerLocation.GetSafeNormal(0.0001f); // Tolerance�� 0.0001�� ����
	float LocationLength = PlayerLocation.Size();
	float ScaleValue = FMath::Max(0, (LocationLength - 9000.0f) / 5000.0f);

	FVector WorldDirection = FVector(NormalizedLocation.X, NormalizedLocation.Y, 0) * -1.0f;
	// �̵� �Է� �߰�
	AddMovementInput(WorldDirection, ScaleValue);


	EdgeMode();
}

void AUSCropoutPlayer::EdgeMode()
{
	//FVector WorldDirection;
	//float ScaleValue;

	FVector2D ViewportCenter = GetViewportCenter();

	FVector2D ScreenPos;
	FVector Intersection;
	ProjectMouseToGroundPlane(ScreenPos, Intersection);

	FVector Direction = CursorDistFromViewportCenter(ScreenPos - ViewportCenter);
	float Strength = 1.0f;

	FTransform ActorTransform = GetActorTransform();
	FVector TransformDirection = UKismetMathLibrary::TransformDirection(ActorTransform, Direction);

	AddMovementInput(TransformDirection, Strength);
}

void AUSCropoutPlayer::ProjectMouseToGroundPlane(FVector2D& ScreenPosition, FVector& Intersection)
{
	bool bMousePostion;
	ScreenPosition = GetMouseViewportPosition(bMousePostion);
	if (bMousePostion == false)
		ScreenPosition = GetViewportCenter();

	Intersection = ProjectScreenPositionToGamePlane(ScreenPosition);
}

FVector2D AUSCropoutPlayer::GetViewportCenter()
{
	APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	if (PlayerController == nullptr)
		return FVector2D::ZeroVector;

	int32 ViewportSizeX, ViewportSizeY;
	PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);

	return FVector2D(ViewportSizeX / 2.0f, ViewportSizeY / 2.0f);
}

FVector2D AUSCropoutPlayer::GetMouseViewportPosition(bool& bMousePostion)
{
	APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	if (PlayerController == nullptr)
		return FVector2D::ZeroVector;

	float LocationX, LocationY;
	bMousePostion = PlayerController->GetMousePosition(LocationX, LocationY);

	return FVector2D(LocationX, LocationY);
}

FVector AUSCropoutPlayer::ProjectScreenPositionToGamePlane(FVector2D ScreenPosition)
{
	APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	if (PlayerController == nullptr)
		return FVector::ZeroVector;

	FVector WorldPosition, WorldDirection;
	PlayerController->DeprojectScreenPositionToWorld(ScreenPosition.X, ScreenPosition.Y, WorldPosition, WorldDirection);

	FVector LineEnd = WorldPosition + (WorldDirection * 100000.0f);
	FPlane APlane(FVector(0.0f, 0.0f, 0.0f), FVector(0.0f, 0.0f, 1.0f));

	FVector Intersection;
	float T;
	UKismetMathLibrary::LinePlaneIntersection(WorldPosition, LineEnd, APlane, T, Intersection);

	return Intersection;
}

FVector AUSCropoutPlayer::CursorDistFromViewportCenter(FVector2D ScreenPos)
{
	FVector2D Distance = CalculateEdgeMoveDistance();
	FVector2D OffsetMousePosition = OffsetMousePositionToCreateDeadZone(ScreenPos, Distance);
	FVector2D Adjusted = AdjustForNegativeDirection(ScreenPos);

	return FVector(Adjusted.Y * OffsetMousePosition.Y * -1.0f, Adjusted.X * OffsetMousePosition.X, 0.0f);
}

FVector2D AUSCropoutPlayer::CalculateEdgeMoveDistance()
{
	FVector2D ViewportCenter = GetViewportCenter();

	// ���� Edge Move Distance ���
	return FVector2D(ViewportCenter.X - EdgeMoveDistance, ViewportCenter.Y - EdgeMoveDistance);
}

FVector2D AUSCropoutPlayer::OffsetMousePositionToCreateDeadZone(FVector2D ScreenPos, FVector2D Distance)
{
	// ���콺 ��ġ�� ���밪 ���
	float AbsMouseX = FMath::Abs(ScreenPos.X);
	float AbsMouseY = FMath::Abs(ScreenPos.Y);

	// X��, Y�࿡�� Dead Zone�� ������ ��ġ ���
	float OffsetX = FMath::Max(0.0f, AbsMouseX - Distance.X) / EdgeMoveDistance;
	float OffsetY = FMath::Max(0.0f, AbsMouseY - Distance.Y) / EdgeMoveDistance;

	return FVector2D(OffsetX, OffsetY);
}

FVector2D AUSCropoutPlayer::AdjustForNegativeDirection(FVector2D InputVector)
{
	float AdjustedX = FMath::Sign(InputVector.X);
	float AdjustedY = FMath::Sign(InputVector.Y);

	return FVector2D(AdjustedX, AdjustedY);
}

void AUSCropoutPlayer::PositionCollisionOnGroundPlane()
{

	//APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	//if (PlayerController == nullptr)
	//	return;
	//FVector WorldLocation, WorldDirection;
	//if (PlayerController->DeprojectScreenPositionToWorld(ScreenPosition.X, ScreenPosition.Y, WorldLocation, WorldDirection))
	//{
	//	// ������ �������� ����ϱ� ���� ������ �ָ� �ִ� �������� ������ �����մϴ�.
	//	FVector LineEnd = WorldLocation + WorldDirection * 10000.0f;
	//	FVector Intersection = FMath::LinePlaneIntersection(WorldLocation, LineEnd, FVector(0.0f, 0.0f, 0.0f), FVector(0.0f, 0.0f, 1.0f));

	//	// 2. ���� ��ġ�� �����ͼ� Z ���� -500���� ����
	//	FVector CurrentLocation = TargetComponent->GetComponentLocation();
	//	CurrentLocation.Z = -500.0f;

	//	// 3. ������ ��ġ ���
	//	float DeltaTime = GetWorld()->GetDeltaSeconds();
	//	FVector TargetLocation = FMath::VInterpTo(CurrentLocation, Intersection, DeltaTime, 12.0f);

	//	// 4. ��ġ �Է� �Ǵ� ���콺 �Է¿� ���� ��ġ ����
	//	FVector FinalLocation = bIsTouchInput ? TargetLocation : Intersection;

	//	// 5. ��ġ ����
	//	TargetComponent->SetWorldLocation(FinalLocation);
	//}

}