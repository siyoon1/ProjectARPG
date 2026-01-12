// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ItemDataBase.h"
#include "ProjectARPG/Inventory/C_ItemObject.h"

const FS_ItemData* UC_ItemDataBase::findItemData(FName ItemID) const
{
	if (!m_ItemDataTable)
		return nullptr;

	return m_ItemDataTable->FindRow<FS_ItemData>(ItemID, TEXT("findItemData"));
}

UC_ItemObject* UC_ItemDataBase::createItemObject(FName ItemID, UObject* Owner)
{
	if (!Owner)
		return nullptr;

	const FS_ItemData* Data = findItemData(ItemID);
	if (!Data || !Data->ItemClass)
		return nullptr;

	UC_ItemObject* Item = NewObject<UC_ItemObject>(Owner, Data->ItemClass);
	Item->init(Data);
	return Item;
}
