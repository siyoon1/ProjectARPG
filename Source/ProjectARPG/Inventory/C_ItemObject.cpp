// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ItemObject.h"

void UC_ItemObject::init(const FS_ItemData* InData)
{
    check(InData);
    m_ItemData = InData;
}

FText UC_ItemObject::getItemName() const
{
    return m_ItemData ? m_ItemData->ItemName : FText::GetEmpty();
}

UTexture2D* UC_ItemObject::getItemIcon() const
{
    return m_ItemData ? m_ItemData->Icon : nullptr;
}

FName UC_ItemObject::getItemID() const
{
    return m_ItemData ? m_ItemData->ItemID : NAME_None;
}

bool UC_ItemObject::use_Implementation(AC_CombatCharacter* User)
{
    return false;
}
