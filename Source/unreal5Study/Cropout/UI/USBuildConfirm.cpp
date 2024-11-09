// Fill out your copyright notice in the Description page of Project Settings.


#include "Cropout/UI/USBuildConfirm.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Border.h"
#include "Kismet/KismetMathLibrary.h"

void UUSBuildConfirm::NativeConstruct()
{
    Super::NativeConstruct();
}

void UUSBuildConfirm::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

    UpdateBorderPosition(InDeltaTime);

    //if (Border)
    //{
    //    // ���콺 ��ġ�� ������
    //    FVector2D MousePosition;
    //    if (UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetMousePosition(MousePosition.X, MousePosition.Y))
    //    {
    //        // ���콺 ��ġ�� ���� ��ǥ�� ��ȯ
    //        FVector2D LocalMousePosition = UWidgetLayoutLibrary::GetViewportWidgetGeometry(this).AbsoluteToLocal(MousePosition);

    //        // Border ��ġ ������Ʈ
    //        Border->SetRenderTranslation(LocalMousePosition);
    //    }
    //}

    
 
}

FVector2D UUSBuildConfirm::GetClampedScreenPosition()
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController == nullptr || PlayerController->GetPawn() == nullptr)
        FVector2D::ZeroVector;

    AActor* TargetActor = PlayerController->GetPawn();

    // 1. TargetActor�� ���� ��ġ�� ȭ�� ��ǥ�� ��ȯ
    FVector WorldPosition = TargetActor->GetActorLocation();
    FVector2D ScreenPosition;
    bool bIsOnScreen = PlayerController->ProjectWorldLocationToScreen(WorldPosition, ScreenPosition, true);

    // 2. ����Ʈ ũ�� �� ������ ��������
    FVector2D ViewportSize;
    GEngine->GameViewport->GetViewportSize(ViewportSize);
    float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(PlayerController);

    // 3. �������� �����Ͽ� ȭ�� ��ǥ ����
    ViewportSize /= ViewportScale;
    ScreenPosition /= ViewportScale;

    FVector2D ReturnPosition;
    // 4. ȭ�� ��ǥ�� �����Ͽ� ����Ʈ ���� ���� (Clamp)
    ReturnPosition.X = FMath::Clamp(ScreenPosition.X, 150.0f, ViewportSize.X - 150.0f);
    ReturnPosition.Y = FMath::Clamp(ScreenPosition.Y, 0.0f, ViewportSize.Y - 350.0f);

    return ReturnPosition;
}

void UUSBuildConfirm::UpdateBorderPosition(float InDeltaTime)
{
    if (Border == nullptr)
        return;

    FVector2D TargetPosition = GetClampedScreenPosition();
    FVector TargetPosition2 = FVector(TargetPosition.X, TargetPosition.Y, 0);

    UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("X: %.2f, Y: %.2f, Z: %.2f"), TargetPosition2.X, TargetPosition2.Y, TargetPosition2.Z), true, true, FColor::Green, 5.0f);

    // ���� ��ġ ��������
    FVector CurrentPosition(Border->RenderTransform.Translation.X, Border->RenderTransform.Translation.Y, 0.0);

    // Spring Interpolation (Vector Spring Interp)
    FVectorSpringState SpringState;
    float Stiffness = 50.0f;
    float CriticalDampingFactor = 0.9f;
    float Mass = 1.0f;
    float TargetVelocityAmount = 0.75f;

    
    FVector InterpolatedPosition = UKismetMathLibrary::VectorSpringInterp(
        CurrentPosition,                // ���� ��ġ
        TargetPosition2,                // ��ǥ ��ġ
        SpringState,                    // ������ ���� (ĳ�� ����)
        Stiffness,                      // ���� (Stiffness)
        CriticalDampingFactor,          // ���� ����
        InDeltaTime,   // ��Ÿ Ÿ��
        Mass,                           // ����
        TargetVelocityAmount            // ��ǥ �ӵ� ����
    );

    // Border�� ��ġ ������Ʈ
    FWidgetTransform Transform;
    Transform.Translation = FVector2D(InterpolatedPosition.X, InterpolatedPosition.Y);
    Transform.Scale = FVector2D(1.0f, 1.0f);
    Transform.Shear = FVector2D(0.0f, 0.0f);
    Transform.Angle = 0.0f;

    // ��ȯ ����
    Border->SetRenderTransform(Transform);
}