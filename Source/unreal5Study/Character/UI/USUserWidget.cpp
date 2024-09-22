// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UI/USUserWidget.h"
#include "WidgetGameInstance.h"


void UUSUserWidget::SetOwningActor(AActor* InOwner)
{
	OwningActor = InOwner;
}

void UUSUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

    if (UWidgetGameInstance* WidgetGameInstance = UGameInstance::GetSubsystem<UWidgetGameInstance>(GetGameInstance()))
    {
        // �����ϸ� �ٽ� ��������
        if (WidgetGameInstance->WidgetMessageMap.Contains(static_cast<uint32>(WidgetID)))
        {
            if (WidgetGameInstance->WidgetMessageMap[static_cast<uint32>(WidgetID)].IsAlreadyBound(this, &ThisClass::DelegateMessage) == false)
            {
                WidgetGameInstance->WidgetMessageMap[static_cast<uint32>(WidgetID)].AddDynamic(this, &ThisClass::DelegateMessage);
            }
        }
        else
        {
            FOnWidgetMessage WidgetMessageDelegate;
            WidgetMessageDelegate.AddDynamic(this, &ThisClass::DelegateMessage);

            WidgetGameInstance->WidgetMessageMap.Add(static_cast<uint32>(WidgetID), WidgetMessageDelegate);
        }
    }
    
    // Ŀ���� ���̰� �����ϰ�, ���Ӱ� UI ��� �Է� �����ϰ� ����
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        PC->bShowMouseCursor = true;

        FInputModeGameAndUI InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        InputMode.SetWidgetToFocus(TakeWidget()); // ���� ������ ��Ŀ���� ����

        PC->SetInputMode(InputMode);
    }
}

void UUSUserWidget::NativeDestruct()
{
	Super::NativeDestruct();

    // Ŀ���� ����� ����
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
    }
}

void UUSUserWidget::DelegateMessage(int32 MessageType, UWidgetMessage* WidgetMessage)
{
    ResponseMessage(MessageType, WidgetMessage);
}

void UUSUserWidget::ResponseMessage(int32 MessageType, UWidgetMessage* WidgetMessage)
{
}

void UUSUserWidget::SendMessage(EWidgetID SendWidgetID, int32 MessageType, UWidgetMessage* WidgetMessage)
{
	if (UWidgetGameInstance* WidgetGameInstance = UGameInstance::GetSubsystem<UWidgetGameInstance>(GetGameInstance()))
	{
		if (WidgetGameInstance->WidgetMessageMap.Contains(static_cast<uint32>(SendWidgetID)))
		{
			WidgetGameInstance->WidgetMessageMap[static_cast<uint32>(SendWidgetID)].Broadcast(MessageType, WidgetMessage);
		}
	}
}
