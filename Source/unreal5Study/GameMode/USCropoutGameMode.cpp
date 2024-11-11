// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/USCropoutGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "../Cropout/Island/USSpawner.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/LatentActionManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include <Kismet/KismetMathLibrary.h>
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Kismet/KismetMathLibrary.h"

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
    SpawnVillager();
    SpawnInteractables();
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

FVector AUSCropoutGameMode::GetRandomPointInBounds()
{
    FVector Origin, BoxExtent;
    TownHall->GetActorBounds(false, Origin, BoxExtent); 

    FVector RandomUnitVector = UKismetMathLibrary::RandomUnitVector();

    FVector CalcVector = (RandomUnitVector * FMath::Min(BoxExtent.X, BoxExtent.Y) * 2.0f) + Origin;
    CalcVector.Z = 0;

    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    FNavLocation NavLocation;
    if (NavSystem && NavSystem->GetRandomReachablePointInRadius(CalcVector, 500.0f, NavLocation))
    {
        return NavLocation.Location;
    }

    return FVector::ZeroVector;
}

void AUSCropoutGameMode::SpawnVillager()
{
    if (VillagerRef == nullptr)
        return;

    for (int32 i = 0; i < 3; i++)
    {
        FVector SpawnLocation = GetRandomPointInBounds();
        FRotator SpawnRotation = FRotator::ZeroRotator;

        SpawnLocation.Z += 92.561032;

        APawn* SpawnedVillager = UAIBlueprintHelperLibrary::SpawnAIFromClass(
            GetWorld(),
            VillagerRef,
            nullptr,            // AI�� ������ Behavior Tree
            SpawnLocation,
            SpawnRotation,
            true                // true�� �����ϸ� �浹�� ��� �������� �ʰ� ��ó�� ���� �õ�
        );

        if(SpawnedVillager)
            VillagerCount++;
    }
    
    OnUpdateVillagers.Broadcast(VillagerCount);
}

void AUSCropoutGameMode::SpawnInteractables()
{
    ReadyToSpawn();
}

void AUSCropoutGameMode::ReadyToSpawn()
{
    int32 IndexCounter = 0;
    // Ÿ�̸� ���� (0.5�ʸ��� �׺���̼� ���� ���� Ȯ��)
    GetWorldTimerManager().SetTimer(NavCheckHandle, this, &AUSCropoutGameMode::CheckNavigationBuild, 0.5f, true, -0.5f);
}

void AUSCropoutGameMode::CheckNavigationBuild()
{
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

    if (NavSys && !NavSys->IsNavigationBuildingLocked())
    {
        // �׺���̼� ���尡 �Ϸ�� ��� Ÿ�̸� ����
        GetWorldTimerManager().ClearTimer(NavCheckHandle);

        // ���� ���� ���� (��: ���� �۾�)
        OnLoadCompleted.Broadcast();
    }
}
