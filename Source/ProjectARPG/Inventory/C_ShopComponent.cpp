// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ShopComponent.h"
#include "ProjectARPG/Inventory/C_Inventory.h"
#include "ProjectARPG/ActorComponents/C_CurrencyComponent.h"

// Sets default values for this component's properties
UC_ShopComponent::UC_ShopComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	// ...
}


// Called when the game starts
void UC_ShopComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

const FS_ShopItem* UC_ShopComponent::findShopItem(FName ItemID) const
{
	for (const FS_ShopItem& item : m_ItemsForSale)
	{
		if (item.ItemID == ItemID)
			return &item;
	}

	return nullptr;
}


// Called every frame
void UC_ShopComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UC_ShopComponent::buyItem(FName ItemID, int32 nCount, UC_Inventory* PlayerInventory, UC_CurrencyComponent* CurrencyComp)
{
	if (!PlayerInventory || !CurrencyComp || nCount <= 0)
		return false;

	const FS_ShopItem* ShopItem = findShopItem(ItemID);
	if (!ShopItem)
		return false;

	int32 nTotalPrice = ShopItem->Price * nCount;

	if (!CurrencyComp->spendGold(nTotalPrice))
		return false;

	if (!PlayerInventory->addItemByID(ItemID, nCount))
	{
		// 실패 시 롤백
		CurrencyComp->addGold(nTotalPrice);
		return false;
	}

	return true;
}

bool UC_ShopComponent::sellItem(FName ItemID, int32 nCount, UC_Inventory* PlayerInventory, UC_CurrencyComponent* CurrencyComp)
{
	if (!PlayerInventory || !CurrencyComp || nCount <= 0)
		return false;

	if (!PlayerInventory->hasItem(ItemID, nCount))
		return false;

	const FS_ShopItem* ShopItem = findShopItem(ItemID);
	if (!ShopItem)
		return false;

	int32 nTotalGold = (ShopItem->Price / 2) * nCount;

	if (!PlayerInventory->removeItem(ItemID, nCount))
		return false;

	CurrencyComp->addGold(nTotalGold);

	return true;
}

