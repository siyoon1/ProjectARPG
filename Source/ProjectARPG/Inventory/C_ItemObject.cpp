// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ItemObject.h"

FText UC_ItemObject::getItemName() const
{
    return m_ItemName;
}

UTexture2D* UC_ItemObject::getItemIcon() const
{
    return m_Icon;
}
