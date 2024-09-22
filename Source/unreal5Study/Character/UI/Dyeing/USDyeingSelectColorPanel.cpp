// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UI/Dyeing/USDyeingSelectColorPanel.h"
#include "USDyeingData.h"
#include "USDyeingColorDetail.h"

UUSDyeingSelectColorPanel::UUSDyeingSelectColorPanel(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    WidgetID = EWidgetID::DyeingSelectColorPanel;
}


void UUSDyeingSelectColorPanel::ResponseMessage(int32 MessageType, UWidgetMessage* WidgetMessage)
{
    UDyeingMessage* DyeingMessage = Cast<UDyeingMessage>(WidgetMessage);
    switch (MessageType)
    {
    case 0: // ����
        WBP_DyeingColorDetail_Origin->SetColor();
        break;
    case 1: // ������ ����
        WBP_DyeingColorDetail_Select->SetColor();
        break;
    case 2: // �������ϳ� ���콺 ������ ����Ⱑ �����Ҵ�....
        WBP_DyeingColorDetail_Random->SetColor();
        break;
    default:
        break;
    }
}