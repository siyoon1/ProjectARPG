// Fill out your copyright notice in the Description page of Project Settings.


#include "C_QuickSlotComponent.h"
#include "ProjectARPG/Inventory/C_Inventory.h"

// Sets default values for this component's properties
UC_QuickSlotComponent::UC_QuickSlotComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UC_QuickSlotComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UC_QuickSlotComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UC_QuickSlotComponent::assignItemToQuickSlot(FName ItemID, int32 nSlotIndex)
{
	if (!m_Inventory)
		return false;
	
	if (nSlotIndex < 0 || nSlotIndex >= m_MaxQuickSlotCount)
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

