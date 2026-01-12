// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ARPGGameInstance.h"

void UC_ARPGGameInstance::Init()
{
	Super::Init();

	if (!m_ItemDBClass)
	{
		UE_LOG(LogTemp, Error, TEXT("ItemDBClass is NOT set"));
		return;
	}

	ItemDB = NewObject<UC_ItemDataBase>(this, m_ItemDBClass);

	if (!ItemDB)
	{
		UE_LOG(LogTemp, Error, TEXT("ItemDB creation failed"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ItemDB initialized successfully"));
}
