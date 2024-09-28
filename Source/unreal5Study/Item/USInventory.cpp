// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/USInventory.h"
#include "../Data/ModularCharacterDataSubsystem.h"
#include "USItemData.h"

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
}

void UUSInventory::BuildItemList()
{
	// �ʱ� ����
	if (AActor* Owner = GetOwner())
	{
		// �ʱ⿡ ������ ����Ϸ��� ��ⷯ�� ������µ�.. ��¼�ٺ��� �������� ����ϴµ�...
		// ������ ������ ���Ǹ� ���� �ؾ��� �� ����
		if (UModularCharacterDataSubsystem* ModularSubsystem = UGameInstance::GetSubsystem<UModularCharacterDataSubsystem>(Owner->GetWorld()->GetGameInstance()))
		{
			int32 Row = 0, Column = 0;
			TArray<FModularCharacterRaw> ModularArray;
			ModularSubsystem->GetModularList(ModularArray);
			for (const auto& Modular : ModularArray)
			{
				if (Modular.ModularIcon != nullptr) // �������� �ִ°͸�.. ���߿� ���۾��̶�..
				{
					UUSItemData* item = NewObject<UUSItemData>();
					item->ItemName = Modular.ModularName;
					item->ItemMesh = Modular.ModularMesh;
					item->ItemIcon = Modular.ModularIcon;

					for (int8 i = 0; i < static_cast<int8>(EModularColorParts::MAX); i++)
					{
						FLinearColor LinearColor = ModularSubsystem->GetColor(Modular, i);
						item->ItemOriginPartsColor.Add(i, LinearColor);
						item->ItemChangePartsColor.Add(i, LinearColor); // �ʱ⿡�� ������ ���� �� ����
					}

					Itemlist.Add(item);
				}
			}
		}
	}
}

