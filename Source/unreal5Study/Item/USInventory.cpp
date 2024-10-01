// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/USInventory.h"

#include "USItemData.h"
#include "USItemGameInstanceSubsystem.h"
#include "NativeGameplayTags.h"
#include "Lyra/GameFramework/GameplayMessageSubsystem.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_Inventory_Update_Color_Message, "Inventory.Update.Color");

// Sets default values for this component's properties
UUSInventory::UUSInventory()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UUSInventory::BeginPlay()
{
	Super::BeginPlay();

	BuildItemList();

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageSubsystem.RegisterListener(TAG_Inventory_Update_Color_Message, this, &ThisClass::ResponseMessage);
}

void UUSInventory::BuildItemList()
{
	// �ʱ� ����
	if (AActor* Owner = GetOwner())
	{
		// �ʱ⿡ ������ ����Ϸ��� ��ⷯ�� ������µ�.. ��¼�ٺ��� �������� ����ϴµ�...
		// ������ ������ ���Ǹ� ���� �ؾ��� �� ����
		if (UUSItemGameInstanceSubsystem* ItemSubsystem = UGameInstance::GetSubsystem<UUSItemGameInstanceSubsystem>(Owner->GetWorld()->GetGameInstance()))
		{
			TArray<FUSItemData> ItemArray;
			ItemSubsystem->GetItemList(ItemArray);
			for (const auto& ItemData : ItemArray)
			{
				if (ItemData.ItemIcon != nullptr) // �������� �ִ°͸�.. ���߿� ���۾��̶�..
				{
					Itemlist.Add(ItemData);
				}
			}
		}
	}
}

void UUSInventory::ResponseMessage(FGameplayTag Channel, const FDyeingMessageData& Payload)
{
	switch (Payload.MessageType)
	{
	case 0:
	{
		FUSItemData* FoundItem = Itemlist.FindByPredicate([&](const FUSItemData& Item) {
			return Item.ItemName == Payload.ItemData.ItemName;
			});

		if (FoundItem == nullptr)
			return;

		*FoundItem = Payload.ItemData;
		break;
	}
	default:
		break;
	}

}

