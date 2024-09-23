// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/UI/Dyeing/USDyeingPalette.h"
#include "Components/Border.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Math/Vector.h"
#include "Engine/Texture2D.h"
#include "Rendering/Texture2DResource.h"
#include "Components/Image.h"
#include "USDyeingData.h"

// HSV to RGB ��ȯ �Լ� ����
FLinearColor HSVtoRGB(float H, float S, float V)
{
    FVector K(1.0f, 2.0f / 3.0f, 1.0f / 3.0f);

    FVector FracHSV(
        FMath::Frac(H + K.X),
        FMath::Frac(H + K.Y),
        FMath::Frac(H + K.Z)
    );

    FVector p = FracHSV * 6.0f - FVector(3.0f, 3.0f, 3.0f);
    p = p.GetAbs();

    FVector LerpResult(
        FMath::Lerp(K.X, FMath::Clamp(p.X - K.X, 0.0f, 1.0f), S),
        FMath::Lerp(K.Y, FMath::Clamp(p.Y - K.Y, 0.0f, 1.0f), S),
        FMath::Lerp(K.Z, FMath::Clamp(p.Z - K.Z, 0.0f, 1.0f), S)
    );

    FVector RGB = V * LerpResult;
    return FLinearColor(RGB.X, RGB.Y, RGB.Z, 1.0f);
}

void ChangeTexturePixel(UTexture2D* Texture, TArray<FColor> Pixels, int32 Width, int32 Height)
{
    // Mip ������ ����
    FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];

    void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
    FColor* FormData = reinterpret_cast<FColor*>(Data);  // ���ҽ� �����͸� FColor �迭�� ĳ����

    if (FormData)
    {
        for (int32 Y = 0; Y < Height; ++Y)
        {
            for (int32 X = 0; X < Width; ++X)
            {
                int32 Index = Y * Width + X;
                FormData[Index] = Pixels[Index];
            }
        }
    }

    // ������ ��� ����
    Mip.BulkData.Unlock();

    // �ؽ�ó ���ҽ� ������Ʈ
    Texture->UpdateResource();
}

// �׶��̼� �ؽ�ó ���� �Լ�
UTexture2D* UUSDyeingPalette::CreateGradationTexture(UObject* Outer, int32 Width, int32 Height)
{
    // �� �ؽ�ó ����
    UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
    if (!Texture)
    {
        return nullptr;
    }

    // �ؽ�ó ����
    Texture->MipGenSettings = TMGS_NoMipmaps;
    Texture->CompressionSettings = TC_VectorDisplacementmap;
    Texture->SRGB = true;
    Texture->AddToRoot(); // ������ �÷��� ����
    Texture->UpdateResource();

    // �ؽ�ó ������ ���� ����
    Pixels.SetNum(Width * Height);

    // �׶��̼� ����: ���� �������� HSV Hue �� ��ȭ
    for (int32 Y = 0; Y < Height; ++Y)
    {
        for (int32 X = 0; X < Width; ++X)
        {
            float H = static_cast<float>(X) / static_cast<float>(Width); // Hue ���� 0���� 1 ����
            float S = 1.0f; // ä�� ����
            float V = 1.0f; // �� ����

            // HSV -> RGB ��ȯ
            FLinearColor LinearColor = HSVtoRGB(H, S, V);
            FColor Color = LinearColor.ToFColor(false);

            Pixels[Y * Width + X] = LinearColor.ToFColor(false);
        }
    }

    // �ؽ�ó ������Ʈ
    ChangeTexturePixel(Texture, Pixels, Width, Height);

    return Texture;
}

UUSDyeingPalette::UUSDyeingPalette(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    WidgetID = EWidgetID::DyeingPalette;
}

void UUSDyeingPalette::NativeConstruct()
{
	Super::NativeConstruct();

    if (Palette && ColorMaterial)
    {
        // ��Ƽ���� �ν��Ͻ��� ����
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(ColorMaterial, this);
        //DynamicMaterial->
        UTexture2D* GradationTexture = CreateGradationTexture(this, TextureSize, TextureSize);
        DynamicMaterial->SetTextureParameterValue("Palette", GradationTexture);
        // Border�� ��Ƽ���� ����
        Palette->SetBrushFromMaterial(DynamicMaterial);
    }
}

FReply UUSDyeingPalette::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
    // ���콺 Ŭ�� �̺�Ʈ ó��
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        FVector2D ClickedPosition = InMouseEvent.GetScreenSpacePosition();
        UE_LOG(LogTemp, Log, TEXT("Mouse Clicked at: X=%f, Y=%f"), ClickedPosition.X, ClickedPosition.Y);
  
        // ���콺 �̺�Ʈ���� ȭ�� ��ǥ ��������
        FVector2D ScreenSpacePosition = InMouseEvent.GetScreenSpacePosition();

        // ȭ�� ��ǥ�� ���� ���� ��ǥ�� ��ȯ
        FVector2D LocalWidgetPosition = InGeometry.AbsoluteToLocal(ScreenSpacePosition);

        FColor NewColor = GetPixelColor(LocalWidgetPosition.X, LocalWidgetPosition.Y);

        //FLinearColor LinearColor = NewColor.ReinterpretAsLinear();
        //SelectColor->SetColorAndOpacity(LinearColor);

        ChangeModulPartsColor(NewColor);
    }

    return FReply::Handled();
}

// �ؽ�ó���� �ȼ��� ������ �о���� �Լ�
FColor UUSDyeingPalette::GetPixelColor(int32 X, int32 Y)
{
    if (X < 0 || Y < 0 || X >= TextureSize || Y >= TextureSize)
        return FColor::Black; // ��ȿ���� ���� ��� �⺻ ������ ��ȯ

    return Pixels[Y * TextureSize + X];
}

void UUSDyeingPalette::ChangeModulPartsColor(FColor Color)
{
    UDyeingMessage* WidgetMessage = NewObject<UDyeingMessage>();
    WidgetMessage->Color = Color.ReinterpretAsLinear();
    SendMessage(EWidgetID::DyeingPanel, 2, WidgetMessage);
}