// Fill out your copyright notice in the Description page of Project Settings.


#include "C_Inventory.h"

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
	

}


// Called every frame
void UC_Inventory::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UC_Inventory::addItem(UC_ItemObject* pItem, int32 nCount)
{
	if (!pItem || nCount <= 0)
		return false;

	int32 nRemaining = nCount;


	// 기존 슬롯 순회
	for (FS_InventorySlot& slot : m_Slots)
	{
		if (!slot.Item)
			continue;

		if (slot.Item->m_ItemID == pItem->m_ItemID &&
			pItem->m_MaxStack > 1 && slot.nCount < pItem->m_MaxStack)
		{
			int32 nSpace = pItem->m_MaxStack - slot.nCount;
			int32 nAdd = FMath::Min(nSpace, nRemaining);

			slot.nCount += nAdd;
			nRemaining -= nAdd;

			UE_LOG(LogTemp, Warning,
				TEXT("[Inventory] Stack %s +%d (%d/%d)"),
				*pItem->m_ItemID.ToString(),
				nAdd,
				slot.nCount,
				pItem->m_MaxStack
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
		newSlot.Item = pItem;

		int32 nAdd = (pItem->m_MaxStack > 1) ? FMath::Min(pItem->m_MaxStack, nRemaining) : 1;

		newSlot.nCount = nAdd;

		nRemaining -= nAdd;

		m_Slots.Add(newSlot);

		UE_LOG(LogTemp, Warning,
			TEXT("[Inventory] New Slot %s x%d"),
			*pItem->m_ItemID.ToString(),
			nAdd
		);
	}

	m_onInventoryChanged.Broadcast();

	return true;
}

bool UC_Inventory::removeItem(FName ItemID, int32 nCount)
{
	if (nCount <= 0)
		return false;

	int32 nRemaining = nCount;


	for (FS_InventorySlot& slot : m_Slots)
	{
		if (!slot.Item)
			continue;

		if (slot.nCount < 0)
			return false;


		if (ItemID == slot.Item->m_ItemID)
		{
			int32 nSpace = slot.Item->m_MaxStack + nCount;

		}
	}


	return false;
}

TArray<FS_InventorySlot>& UC_Inventory::getSlots()
{
	return m_Slots;
}

