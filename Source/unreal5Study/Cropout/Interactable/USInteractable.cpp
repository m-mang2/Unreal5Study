// Fill out your copyright notice in the Description page of Project Settings.


#include "Cropout/Interactable/USInteractable.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
AUSInteractable::AUSInteractable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AUSInteractable::BeginPlay()
{
	Super::BeginPlay();
	
    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget = this; // �ݹ� ���
    LatentInfo.ExecutionFunction = "AfterDelay"; // ���� �� ȣ���� �Լ� �̸�
    LatentInfo.Linkage = 0; // Latent Action�� ���� ID
    LatentInfo.UUID = 1; // Unique ID

    // DelayUntilNextTick ȣ��
    UKismetSystemLibrary::DelayUntilNextTick(this, LatentInfo);
}
