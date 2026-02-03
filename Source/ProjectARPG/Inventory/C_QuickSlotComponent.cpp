// Fill out your copyright notice in the Description page of Project Settings.


#include "C_QuickSlotComponent.h"
#include "ProjectARPG/Inventory/C_Inventory.h"

UC_QuickSlotComponent::UC_QuickSlotComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UC_QuickSlotComponent::BeginPlay()
{
	Super::BeginPlay();

	m_QuickSlots.SetNum(m_MaxQuickSlotCount);

	for (int32 i = 0; i < m_QuickSlots.Num(); ++i)
	{
		m_QuickSlots[i].slotIndex = i;
		m_QuickSlots[i].ItemID = NAME_None;
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn)
	{
		m_Inventory = OwnerPawn->FindComponentByClass<UC_Inventory>();
	}
	
}


// Called every frame
void UC_QuickSlotComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

bool UC_QuickSlotComponent::assignItemToQuickSlot(FName ItemID, int32 nSlotIndex)
{
	if (!m_Inventory)
		return false;

	if (!m_QuickSlots.IsValidIndex(nSlotIndex))
		return false;

	if (!m_Inventory->hasItem(ItemID, 1))
		return false;

	m_QuickSlots[nSlotIndex].ItemID = ItemID;
	m_QuickSlots[nSlotIndex].slotIndex = nSlotIndex;

	return true;
}

void UC_QuickSlotComponent::useQuickSlot(int32 nSlotIndex)
{
	if (!m_Inventory)
		return;

	if (!m_QuickSlots.IsValidIndex(nSlotIndex))
		return;

	FName ItemID = m_QuickSlots[nSlotIndex].ItemID;

	if (ItemID.IsNone())
		return;

	if (!m_Inventory->removeItem(ItemID, 1))
		return;

	//아이템 효과 실행
	
}

