// Fill out your copyright notice in the Description page of Project Settings.


#include "Cropout/PlayerControll/USCropoutController.h"

void AUSCropoutController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // ���콺 Ŀ���� ����Ʈ�� �������� �ʵ��� ����
   // SetInputMode(InputMode); // �Է� ��� ����
    //bShowMouseCursor = true; // ���콺 Ŀ�� ǥ��
}
