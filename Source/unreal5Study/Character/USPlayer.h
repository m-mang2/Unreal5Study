// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/USCharacterBase.h"
#include "InputActionValue.h"
#include "Character/Input/USInputContextData.h"
#include "Character/Camera/USCameraData.h"
#include "USPlayer.generated.h"



UENUM(BlueprintType)
enum class EInputKey : uint8
{
	None UMETA(DisplayName = "None"),
	Look UMETA(DisplayName = "Look"),
	Move UMETA(DisplayName = "Move"),
	CameraChange UMETA(DisplayName = "CameraChange"),
};

/**
 * 
 */
UCLASS()
class UNREAL5STUDY_API AUSPlayer : public AUSCharacterBase
{
	GENERATED_BODY()

public:
	AUSPlayer();

	virtual void BeginPlay() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// Ư���� ��ǲ ������ �ٲ����� �ִٸ� ���⼭ ���� �� ��
	void SetInputContextChange(class UInputMappingContext* InputMappingContext);
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void CameraChange();

	virtual void SetCameraData(const class UUSCameraData* CameraData);
	EViewType GetNextViewType(EViewType CurrentView);
	
	class UCameraComponent* GetCameraComponent() { return FollowCamera; }
protected:
	// ī�޶�, ���� �ٸ� Ŭ������ ����..
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;

	UPROPERTY(EditAnywhere, Category = CameraTypeMap, Meta = (AllowPrivateAccess = "true"))
	TMap<EViewType, class UUSCameraData*> CameraTypeMap;

	EViewType CurrentViewType = EViewType::FirstPerson;

	// ��ǲ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TMap<EInputKey, TObjectPtr<class UInputAction>> InputActionMap;

	
	// UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HUD)
	TSubclassOf<class UUserWidget> HUDWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = HUD)
	TObjectPtr<class UUserWidget> HUDWidget;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capture)
	TObjectPtr<class USceneCaptureComponent2D> sceneCapture;
};
