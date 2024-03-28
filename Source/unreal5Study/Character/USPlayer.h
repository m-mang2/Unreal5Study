// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/USCharacterBase.h"
#include "InputActionValue.h"
#include "USPlayer.generated.h"

UENUM(BlueprintType)
enum class EViewType : uint8
{
	None UMETA(DisplayName = "None"),
	FirstPerson UMETA(DisplayName = "FirstPerson"),
	ThirdPerson UMETA(DisplayName = "ThirdPerson"),
	TopDown UMETA(DisplayName = "TopDown"),
};

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

	virtual void SetCameraData(const class UUSCameraData* CameraData);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// Ư���� ��ǲ ������ �ٲ����� �ִٸ� ���⼭ ���� �� ��
	void SetInputControll();
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void CameraChange();
	EViewType GetNextViewType(EViewType CurrentView);

protected:
	// ī�޶�, ���� �ٸ� Ŭ������ ����..
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;

	UPROPERTY(EditAnywhere, Category = CameraTypeMap, Meta = (AllowPrivateAccess = "true"))
	TMap<EViewType, class UUSCameraData*> CameraTypeMap;

	EViewType CurrentViewType = EViewType::None;

	// ��ǲ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	TObjectPtr<class UInputMappingContext> InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TMap<EInputKey, TObjectPtr<class UInputAction>> InputActionMap;

	
};
