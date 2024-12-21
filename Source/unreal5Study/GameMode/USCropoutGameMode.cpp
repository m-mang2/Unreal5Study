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
#include "../Lyra/GameFramework/GameplayMessageSubsystem.h"
#include "../Cropout/UI/USResourceUIItem.h"
#include "NavigationPath.h"

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
    OnMonsterHallClassLoaded();
    for (int32 i = 0; i < 3; i++)
    {
        SpawnVillager();
    }

    SendUIResourceValue();
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

void AUSCropoutGameMode::OnMonsterHallClassLoaded()
{
    //MonsterHall

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), SpawnMarkerClass, FoundActors);
    if (FoundActors.Num() <= 0)
        return;

    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (NavSystem == nullptr)
        return;

    FVector TownHallLocation = TownHall->GetActorLocation();
    float MaxDistance = 0.0f;
    AActor* FarthestActor = nullptr;
    for (auto& Actor : FoundActors)
    {
        FVector ActorLocation = Actor->GetActorLocation();

        UNavigationPath* NavPath = NavSystem->FindPathToLocationSynchronously(GetWorld(), TownHallLocation, ActorLocation);
        if (NavPath && NavPath->IsValid())
        {
            float PathLength = NavPath->GetPathLength();
            if (PathLength > MaxDistance)
            {
                MaxDistance = PathLength;
                FarthestActor = Actor;
            }
        }
    }

    if (FarthestActor)
    {
        FVector SpawnLocation = FarthestActor->GetActorLocation();
        FRotator SpawnRotation = FarthestActor->GetActorRotation();

        FActorSpawnParameters SpawnParams;
        AActor* MonsterTownHall = GetWorld()->SpawnActor<AActor>(MonsterHallClass, SpawnLocation, SpawnRotation, SpawnParams);
        if (MonsterTownHall)
        {
            MonsterHall = MonsterTownHall;
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
    
    OnUpdateVillagers.Broadcast(VillagerCount);
}


int32 AUSCropoutGameMode::GetCurrentResources(enum EResourceType Resource)
{
    return Resources[Resource];
}

void AUSCropoutGameMode::RemoveCurrentUILayer()
{
}

void AUSCropoutGameMode::EndGame()
{
}

void AUSCropoutGameMode::SendUIResourceValue()
{
    FCropoutResourceValueMessageData Message;
    Message.Verb = TAG_Cropout_UI_Message;
    Message.Resources = Resources;

    UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
    MessageSystem.BroadcastMessage(Message.Verb, Message);
}

TMap<EResourceType, int32> AUSCropoutGameMode::GetResources()
{
    return Resources;
}

void AUSCropoutGameMode::RemoveTargetResource(EResourceType Resource, int32 Value)
{
    if (Resources.Find(Resource) == nullptr)
        return;

    Resources[Resource] -= Value;

    if (Resources[EResourceType::Food] <= 0)
        EndGame();

    SendUIResourceValue();
}

void AUSCropoutGameMode::AddResource_Implementation(EResourceType Resource, int32 Value)
{
    if (Resources.Find(Resource) == nullptr)
        return;

    Resources[Resource] += Value;

    SendUIResourceValue();
}
