// Fill out your copyright notice in the Description page of Project Settings.


#include "Cropout/Island/USSpawner.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "USSpawner.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "NavigationSystem.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AUSSpawner::AUSSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AUSSpawner::BeginPlay()
{
	Super::BeginPlay();
	AsyncLoadClasses();
}

void AUSSpawner::AsyncLoadClasses()
{
    ClassRefIndex = 0;
    bAsyncComplete = false; 

    if (SpawnTypes.Num() > 0)
    {
        FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
        for (int32 Index = 0; Index < SpawnTypes.Num(); Index++)
        {
            TSoftClassPtr<AActor> ClassRef = SpawnTypes[Index].ClassREf;

            if (ClassRef.IsValid())
            {
                // Ŭ������ �̹� �ε�Ǿ��� ��
                OnAsyncLoadComplete();
            }
            else
            {
                // �񵿱� �ε�
                Streamable.RequestAsyncLoad(ClassRef.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this, &AUSSpawner::OnAsyncLoadComplete));
            }
            //ClassRef->GenerateFunctionList()
        }
    }
    /* �̷� ������� ����ؾ���
    * TSoftClassPtr<AActor> ClassRef = TSoftClassPtr<AActor>(FSoftObjectPath("/Game/Blueprints/BP_Tree.BP_Tree"));

if (ClassRef.IsValid())
{
    // �ڻ��� �̹� �ε�Ǿ� �ִٸ� UClass*�� ��ȯ
    UClass* LoadedClass = ClassRef.Get();

    // UClass*�� ATree�� ����Ŭ�������� Ȯ���ϰ� ĳ����
    if (LoadedClass->IsChildOf(ATree::StaticClass()))
    {
        // �����ϰ� ATree Ŭ������ ��ȯ
        TSubclassOf<ATree> TreeClass = LoadedClass;

        // TreeClass�� �۾� ����
        ATree* SpawnedTree = GetWorld()->SpawnActor<ATree>(TreeClass);
    }
}
else
{
    // �񵿱� �ε�
    ClassRef.LoadAsync(FStreamableDelegate::CreateLambda([=]()
    {
        UClass* LoadedClass = ClassRef.Get();
        if (LoadedClass && LoadedClass->IsChildOf(ATree::StaticClass()))
        {
            // �����ϰ� ATree Ŭ������ ��ȯ
            TSubclassOf<ATree> TreeClass = LoadedClass;

            // TreeClass�� �۾� ����
            ATree* SpawnedTree = GetWorld()->SpawnActor<ATree>(TreeClass);
        }
    }));
}
    */
}

void AUSSpawner::OnAsyncLoadComplete()
{
    ClassRefIndex++;

    // ��� Ŭ������ �ε�Ǿ����� Ȯ��
    if (ClassRefIndex >= SpawnTypes.Num())
    {
        bAsyncComplete = true;
        // ��� Ŭ������ �ε�Ǿ����� ���� ������ ������
        SpawnInstances();
    }
}

void AUSSpawner::SpawnInstances()
{
    SetActorLocation(FVector(0.0f, 0.0f, 0.0f));
    FVector CenterPos = GetRandomPoint();
	for (int32 IndexCounter = 0; IndexCounter < SpawnInstance.Num(); ++IndexCounter)
	{
        UInstancedStaticMeshComponent* InstancedMeshComponent = NewObject<UInstancedStaticMeshComponent>(this);
        InstancedMeshComponent->RegisterComponent();
        InstancedMeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
        //InstancedMeshComponent = Cast<UInstancedStaticMeshComponent>(AddComponent(FName("DynamicMesh"), false, FTransform(), nullptr));

        InstancedMeshComponent->SetStaticMesh(SpawnInstance[IndexCounter].ClassREf);
        PickPointsAroundBiomePoints(InstancedMeshComponent, CenterPos, SpawnInstance[IndexCounter].BiomeScale, SpawnInstance[IndexCounter].BiomeCount, SpawnInstance[IndexCounter].SpawnPerBiome);
        
	}
    //int32 BiomeCount = 3;
    //int32 MaxSpawn = 3;
    //for (int32 IndexCounter = 0; IndexCounter < BiomeCount; ++IndexCounter)
    //{
    //    UInstancedStaticMeshComponent* InstancedMeshComponent = NewObject<UInstancedStaticMeshComponent>(this);
    //    if (InstancedMeshComponent)
    //    {
    //        InstancedMeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
    //        InstancedMeshComponent->RegisterComponent();

    //        UStaticMesh* NewMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Path/To/YourMesh.YourMesh"));
    //        if (NewMesh)
    //        {
    //            InstancedMeshComponent->SetStaticMesh(NewMesh);
    //        }

    //        for (int32 SpawnIndex = 0; SpawnIndex < MaxSpawn; ++SpawnIndex)
    //        {
    //            FTransform InstanceTransform;
    //            InstanceTransform.SetLocation(FVector::ZeroVector);
    //            //InstanceTransform.SetScale3D(FVector(BiomeScale)); 

    //            InstancedMeshComponent->AddInstance(InstanceTransform);
    //        }
    //    }
    //}

    //FActorSpawnParameters SpawnParams;
    //SpawnParams.Owner = this;
    //if (ClassRef)
    //{
    //    for (int32 i = 0; i < BiomeCount; ++i)
    //    {
    //        FVector SpawnLocation = FVector(i * 100.0f, 0.0f, 0.0f); // Adjust spawn location as needed
    //        FRotator SpawnRotation = FRotator::ZeroRotator;

    //        AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ClassRef, SpawnLocation, SpawnRotation, SpawnParams);
    //        if (SpawnedActor)
    //        {

    //        }
    //    }
    //}
}

FVector AUSSpawner::GetRandomPoint()
{
    UWorld * World = GetWorld();
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World);

    if (NavSystem == nullptr)
        return FVector::ZeroVector;
    FVector Origin = { 0.0f, 0.0f, 0.0f };
    float Radius = 1000.0;
    FNavLocation RandomLocation;
    bool bSuccess = NavSystem->GetRandomReachablePointInRadius(Origin, Radius, RandomLocation);
    NavSystem->GetRandomPoint(RandomLocation);
    return bSuccess ? RandomLocation.Location : RandomLocation;
}

void AUSSpawner::PickPointsAroundBiomePoints(class UInstancedStaticMeshComponent* Mesh, FVector BiomeCenter, float Radius, int32 BiomeCount, int32 MaxSpawn)
{
    UWorld* World = GetWorld();

    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World);
    if (NavSystem == nullptr) 
        return;

    FRandomStream RandomStream;
    int32 LoopCount = RandomStream.RandRange(0, MaxSpawn);

    for (int32 i = 0; i < LoopCount; i++)
    {
        FNavLocation RandomLocation;
        bool bSuccess = NavSystem->GetRandomReachablePointInRadius(BiomeCenter, Radius, RandomLocation);

        if (bSuccess)
        {
            FTransform InstanceTransform;
            InstanceTransform.SetLocation(RandomLocation.Location);
            InstanceTransform.SetRotation(FQuat(FRotator(0.f, 0.f, 0.f)));
            InstanceTransform.SetScale3D(FVector(1.f, 1.f, 1.f));
            Mesh->AddInstance(InstanceTransform);
        }
    }
}

FTransform AUSSpawner::GenerateRandomTransform(FVector Pos, FVector SpawnPos, float Radius)
{
    float VectorLength = FVector::Dist(Pos, SpawnPos);
    float ScaleValue = FMath::Lerp(0.8f, 1.5f, VectorLength / Radius);

    FTransform NewTransform;
    NewTransform.SetLocation(FVector(SpawnPos.X, SpawnPos.Y, 0.0));
    NewTransform.SetRotation(FRotator(0.0f, 0.0f, 0.0f).Quaternion());
    NewTransform.SetScale3D(FVector(ScaleValue)); 

    return NewTransform;
}

