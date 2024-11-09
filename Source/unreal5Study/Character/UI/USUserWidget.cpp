// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UI/USUserWidget.h"

void UUSUserWidget::SetOwningActor(AActor* InOwner)
{
	OwningActor = InOwner;
}

void UUSUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
    
    // Ŀ���� ���̰� �����ϰ�, ���Ӱ� UI ��� �Է� �����ϰ� ����
    if (GetWorld())
    {
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            PC->bShowMouseCursor = true;

            FInputModeGameAndUI InputMode;
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            InputMode.SetWidgetToFocus(TakeWidget()); // ���� ������ ��Ŀ���� ����

            PC->SetInputMode(InputMode);
        }
    }
}

void UUSUserWidget::NativeDestruct()
{
	Super::NativeDestruct();

    // Ŀ���� ����� ����
    if (GetWorld())
    {
        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            PC->bShowMouseCursor = false;
            PC->SetInputMode(FInputModeGameOnly());
        }
    }
}

FReply UUSUserWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
    return FReply::Handled();
}

FReply UUSUserWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
    return FReply::Handled();
}
