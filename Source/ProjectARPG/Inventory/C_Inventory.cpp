// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Inventory.h"
#include "ProjectARPG/Inventory/C_ItemDataBase.h"
#include "ProjectARPG/Game/C_ARPGGameInstance.h"

// Sets default values for this component's properties
UC_Inventory::UC_Inventory()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UC_Inventory::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
	UC_ARPGGameInstance* GI = Cast<UC_ARPGGameInstance>(GetWorld()->GetGameInstance());
	if (GI)
	{
		m_ItemDB = GI->getItemDB();
	}
}


// Called every frame
void UC_Inventory::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UC_Inventory::addItem(UC_ItemObject* Item, int32 nCount)
{
	if (!Item || nCount <= 0)
		return false;

	int32 nRemaining = nCount;


	// 기존 슬롯 순회
	for (FS_InventorySlot& slot : m_Slots)
	{
		if (!slot.Item)
			continue;

		if (slot.Item->m_ItemData->ItemID == Item->m_ItemData->ItemID &&
			Item->m_ItemData->MaxStack > 1 && slot.nCount < Item->m_ItemData->MaxStack)
		{
			int32 nSpace = Item->m_ItemData->MaxStack - slot.nCount;
			int32 nAdd = FMath::Min(nSpace, nRemaining);

			slot.nCount += nAdd;
			nRemaining -= nAdd;

			UE_LOG(LogTemp, Warning,
				TEXT("[Inventory] Stack %s +%d (%d/%d)"),
				*Item->m_ItemData->ItemID.ToString(),
				nAdd,
				slot.nCount,
				Item->m_ItemData->MaxStack
			);

			// 나머지 0이면 성공으로 그냥 끝
			if (nRemaining <= 0)
				return true;
			
		}
	}

	// 기존 슬롯에 아이템을 다 못넣은 경우
	while (nRemaining > 0)
	{
		if (m_Slots.Num() >= m_MaxSlotCount)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Inventory] Slot Full"));
			return false;
		}

		FS_InventorySlot newSlot;
		newSlot.Item = Item;

		int32 nAdd = (Item->m_ItemData->MaxStack > 1) ? FMath::Min(Item->m_ItemData->MaxStack, nRemaining) : 1;

		newSlot.nCount = nAdd;

		nRemaining -= nAdd;

		m_Slots.Add(newSlot);

		UE_LOG(LogTemp, Warning,
			TEXT("[Inventory] New Slot %s x%d"),
			*Item->m_ItemData->ItemID.ToString(),
			nAdd
		);
	}

	m_onInventoryChanged.Broadcast();

	return true;
}

bool UC_Inventory::addItemByID(FName ItemID, int32 nCount)
{
	if (!m_ItemDB || nCount < 0)
		return false;

	for (int32 i = 0; i < nCount; ++i)
	{
		UC_ItemObject* NewItem = m_ItemDB->createItemObject(ItemID, this);
		if (!NewItem || !addItem(NewItem, 1))
			return false;
	}

	return true;
}

bool UC_Inventory::removeItem(FName ItemID, int32 nCount)
{
	if (nCount <= 0)
		return false;

	int32 nRemaining = nCount;


	int32 nTotalCount{};

	for (FS_InventorySlot& slot : m_Slots)
	{
		if (!slot.Item)
			continue;

		if (slot.nCount < 0)
			return false;


		if (ItemID == slot.Item->m_ItemData->ItemID)
		{
			nTotalCount += slot.nCount;

		}
	}

	if (nTotalCount < nCount)
		return false;

	for (int32 i = 0; i < m_Slots.Num(); ++i)
	{
		FS_InventorySlot& slot = m_Slots[i];

		if (!slot.Item || slot.Item->m_ItemData->ItemID != ItemID)
			continue;

		int32 nRemove = FMath::Min(slot.nCount, nRemaining);

		slot.nCount -= nRemove;

		nRemaining -= nRemove;

		if (slot.nCount <= 0)
		{
			m_Slots.RemoveAt(i);
			--i;
		}
	}

	
	if (nRemaining <= 0)
	{
		m_onInventoryChanged.Broadcast();
		return true;
	}

	return false;
}

TArray<FS_InventorySlot>& UC_Inventory::getSlots()
{
	return m_Slots;
}

UC_ItemObject* UC_Inventory::findItemObject(FName ItemID)
{
	for (FS_InventorySlot& Slot : m_Slots)
	{
		if (Slot.Item && Slot.Item->getItemID() == ItemID)
		{
			return Slot.Item;
		}
	}

	return nullptr;
}

bool UC_Inventory::hasItem(FName ItemID, int32 nCount)
{
	if (nCount < 0)
		return false;

	int32 nTotalCount{};

	for (const FS_InventorySlot& slot : m_Slots)
	{

		if (!slot.Item)
			continue;

		if (slot.Item->getItemID() == ItemID)
		{
			nTotalCount += slot.nCount;

			if (nTotalCount >= nCount)
				return true;
		}
			
	}

	return false;
}

int32 UC_Inventory::getItemCount(FName ItemID)
{
	for (FS_InventorySlot& Slot : m_Slots)
	{
		if (Slot.Item && Slot.Item->getItemID() == ItemID)
		{
			return Slot.nCount;
		}
	}

	return 0;
}

