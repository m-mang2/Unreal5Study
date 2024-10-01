// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UI/Dyeing/USDyeingPartsSelecter.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "DyeingGameInstance.h"
#include "USDyeingData.h"
#include "NativeGameplayTags.h"
#include "../../../Lyra/GameFramework/GameplayMessageSubsystem.h"

void UUSDyeingPartsSelecter::NativeConstruct()
{
	Super::NativeConstruct();

    int32 Index = 0;
    ButtonList.Empty();
    // VerticalBox�� ��� �ڽ� ������ ��ȸ
    for (UWidget* ChildWidget : VerticalBox->GetAllChildren())
    {
        // �ش� �ڽ��� UButton���� Ȯ��
        UButton* Button = Cast<UButton>(ChildWidget);
        if (Button)
        {
            SaveStyle = Button->GetStyle();
            // ��ư ����Ʈ�� �߰�
            ButtonList.Add(Index++, Button);
            if(Button->OnPressed.IsAlreadyBound(this, &UUSDyeingPartsSelecter::OnTabButtonClicked) == false)
                Button->OnPressed.AddDynamic(this, &UUSDyeingPartsSelecter::OnTabButtonClicked);
        }
    }

    if (ButtonList.Num() > 0)
    {
        SelectButton( 0);
    }
}

void UUSDyeingPartsSelecter::OnTabButtonClicked()
{
    for (int32 i = 0; i < ButtonList.Num(); i++)
    {
        // ��Ÿ���� �����Ұ� �ƴ϶� ��� �̹��� ���� �����߰ڴ�...
        if (ButtonList[i]->IsPressed())
        {
            UE_LOG(LogTemp, Warning, TEXT("TabButton %d clicked"), i);

            SelectButton(i);
        }
        else
        {
            ButtonList[i]->SetStyle(SaveStyle);
        }
    }
}

void UUSDyeingPartsSelecter::SelectButton(uint8 ButtonIndex)
{
    
    UButton* Button = ButtonList[ButtonIndex];
    if (Button == nullptr)
        return;

    FButtonStyle ButtonStyle = SaveStyle;
    FSlateBrush SelectedBrush;
    SelectedBrush.TintColor = FSlateColor(FLinearColor::Black);
    ButtonStyle.SetNormal(SelectedBrush);
    Button->SetStyle(ButtonStyle);


    FDyeingMessageData Message;
    Message.Verb = TAG_DyeingPanel_Message;
    Message.ColorParts = ButtonIndex;
    Message.MessageType = 1;

    UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
    MessageSystem.BroadcastMessage(Message.Verb, Message);
}
