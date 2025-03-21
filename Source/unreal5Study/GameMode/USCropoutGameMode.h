// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "../Cropout/Interactable/USResourceInterface.h"
#include "../Data/USCropoutSaveGame.h"
#include "USCropoutGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FOnUpdateVillagers, int32, VillagerCount);

/**
 * 
 */
UCLASS()
class UNREAL5STUDY_API AUSCropoutGameMode : public AGameModeBase, public IUSResourceInterface
{
	GENERATED_BODY()

public:	
	virtual void BeginPlay() override;

	void GetSpawnRef();

	UFUNCTION()
	void IslandGencomplete();

	void BeginAsyncSpawning();
	UFUNCTION()
	void OnAsyncLoadComplete();
	void OnTownHallClassLoaded();
	void OnMonsterHallClassLoaded();
	
	FVector GetSteppedPosition(const FVector& Position, float StepSize);

	FVector GetRandomPointInBounds(AActor* Hall);
	void SpawnVillager();

	void AddResource_Implementation(enum EResourceType Resource, int32 Value);
	virtual void RemoveTargetResource(enum EResourceType Resource, int32 Value);
	virtual int32 GetCurrentResources(enum EResourceType Resource);
	void RemoveCurrentUILayer();

	void EndGame();

	void SendUIResourceValue();
	TMap<EResourceType, int32> GetResources();

	void SpawnMonster(TSubclassOf<APawn> MonsterClass);

	void UpdateAllInteractables();
	void UpdateAllResources();
	void UpdateAllVillagers();

	void SaveGame();
	bool LoadGame();
	void ClearGame();
	bool IsSaveData();

	int32 GetSeed();

	void BuildSaveData();

	void InitSoundVolume();
protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> SpawnMarkerClass;
	
	UPROPERTY()
	TObjectPtr<class AUSSpawner> SpawnRef;

	UPROPERTY(EditAnywhere)
	TSoftClassPtr<AActor> TownHallRef;

	UPROPERTY(EditAnywhere)
	TObjectPtr<AActor> TownHall;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> MonsterHallClass;

	UPROPERTY(EditAnywhere)
	TObjectPtr<AActor> MonsterHall;

	UPROPERTY(EditAnywhere)
	TSubclassOf<APawn> VillagerRef;

	int32 VillagerCount = 0;
	

	UPROPERTY(EditAnywhere)
	TMap<EResourceType, int32> Resources;

	UPROPERTY(EditAnywhere, BlueprintReadWrite )
    TArray<FSTSaveInteract> Interactables;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FSTVillager> Villagers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SeedValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bGameModeMain = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class USoundControlBus> CropoutMusicPianoVol = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class USoundControlBus> CropoutMusicPercVol = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class USoundControlBus> CropoutMusicStringsDelay = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class USoundBase> SoundBase = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class USoundControlBus> CropoutMusicWinLose = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class USoundControlBus> CropoutMusicStop = nullptr;

public:
	FOnUpdateVillagers OnUpdateVillagers;
};
