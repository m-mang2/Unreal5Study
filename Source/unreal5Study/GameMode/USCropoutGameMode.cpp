// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/USCropoutGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "../Cropout/Island/USSpawner.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/LatentActionManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

void AUSCropoutGameMode::BeginPlay()
{
	Super::BeginPlay();
    GetSpawnRef();
}

void AUSCropoutGameMode::GetSpawnRef()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ActorClass, FoundActors);

    if (FoundActors.Num() > 0)
    {
        SpawnRef = Cast<AUSSpawner>(FoundActors[0]);
    }
}

void AUSCropoutGameMode::IslandGencomplete()
{
    BeginAsyncSpawning(); 
}

void AUSCropoutGameMode::BeginAsyncSpawning()
{
    FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
    Streamable.RequestAsyncLoad(TownHallRef.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this, &AUSCropoutGameMode::OnAsyncLoadComplete));
}

void AUSCropoutGameMode::OnAsyncLoadComplete()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), SpawnMarkerClass, FoundActors);

    if (FoundActors.Num() > 0)
    {
        SpawnRef = Cast<AUSSpawner>(FoundActors[0]);
    }

    OnTownHallClassLoaded();
}

void AUSCropoutGameMode::OnTownHallClassLoaded()
{
    if (TownHallRef.IsValid() == false)
        return;

    // �ε�� Ŭ���� Ȯ�� �� ĳ����
    UClass* TownHallClass = TownHallRef.Get();
    if (TownHallClass == nullptr)
        return;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), SpawnMarkerClass, FoundActors);

    if (FoundActors.Num() > 0)
    {
        // �������� ���� ��Ŀ ����
        AActor* RandomMarker = FoundActors[FMath::RandRange(0, FoundActors.Num() - 1)];

        // ���õ� ��Ŀ ��ġ ��������
        FVector SpawnLocation = RandomMarker->GetActorLocation();
        FRotator SpawnRotation = RandomMarker->GetActorRotation();

        FVector NewPosition = GetSteppedPosition(SpawnLocation, 200.0);

        // Ÿ�� Ȧ ���� ����
        FActorSpawnParameters SpawnParams;
        AActor* SpawnedTownHall = GetWorld()->SpawnActor<AActor>(TownHallClass, NewPosition, SpawnRotation, SpawnParams);

        // ������ Ÿ�� Ȧ�� ������ ����
        if (SpawnedTownHall)
        {
            TownHall = SpawnedTownHall;
        }

    }
}

FVector AUSCropoutGameMode::GetSteppedPosition(const FVector& Position, float StepSize)
{
    // Position ������ X�� Y�� StepSize�� ���� �ݿø�
    float SteppedX = FMath::RoundToFloat(Position.X / StepSize) * StepSize;
    float SteppedY = FMath::RoundToFloat(Position.Y / StepSize) * StepSize;

    // ���ο� ���� ��ȯ (Z�� 0���� ����)
    return FVector(SteppedX, SteppedY, 0.0f);
}