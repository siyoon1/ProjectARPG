// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ARPGGameInstance.h"
#include "ProjectARPG/Character/C_PlayerCharacter.h"
#include "ProjectARPG/Actor/C_BonfireActor.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectARPG/Game/C_SaveGame.h"
#include "EngineUtils.h"

void UC_ARPGGameInstance::Init()
{
	Super::Init();

	m_SaveGame = nullptr;

	if (!m_ItemDBClass)
	{
		UE_LOG(LogTemp, Error, TEXT("ItemDBClass is NOT set"));
		return;
	}

	m_ItemDB = NewObject<UC_ItemDataBase>(this, m_ItemDBClass);

	if (!m_ItemDB)
	{
		UE_LOG(LogTemp, Error, TEXT("ItemDB creation failed"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ItemDB initialized successfully"));
}

void UC_ARPGGameInstance::setLastBonfireID(FName NewID)
{
	m_LastBonfireID = NewID;
}

FName UC_ARPGGameInstance::getLastBonfireID() const
{
	return m_LastBonfireID;
}

void UC_ARPGGameInstance::saveGame()
{
	if (!m_SaveGame)
	{
		m_SaveGame = Cast<UC_SaveGame>(UGameplayStatics::CreateSaveGameObject(UC_SaveGame::StaticClass()));
	}

	if (!m_SaveGame)
		return;

	m_SaveGame->m_LastBonfireID = m_LastBonfireID;

	UGameplayStatics::SaveGameToSlot(m_SaveGame, TEXT("ARPG_Save"), 0);
}

void UC_ARPGGameInstance::loadGame()
{
	if (UGameplayStatics::DoesSaveGameExist(TEXT("ARPG_Save"), 0))
	{
		m_SaveGame = Cast<UC_SaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("ARPG_Save"), 0));
	}

	if (m_SaveGame)
	{
		m_LastBonfireID = m_SaveGame->m_LastBonfireID;
	}
}

void UC_ARPGGameInstance::collectPlayerData(AC_PlayerCharacter* Player)
{
	if (!Player || !m_SaveGame)
		return;

	m_SaveGame->m_PlayerHP = Player->getHp();
	//m_SaveGame->m_PlayerMaxHP = Player->getMaxHp();
}

void UC_ARPGGameInstance::applyPlayerData(AC_PlayerCharacter* Player)
{
	if (!Player || !m_SaveGame)
		return;

	//Player->setHp(m_SaveGame->m_PlayerHP);
	//Player->setMaxHp(m_SaveGame->m_PlayerMaxHP);
}

void UC_ARPGGameInstance::respawnPlayer(AC_PlayerCharacter* Player)
{
	if (!Player)
		return;

	loadGame();

	UWorld* World = GetWorld();
	if (!World)
		return;

	for (TActorIterator<AC_BonfireActor> iter(World); iter; ++iter)
	{
		if (iter->getBonfireID() == m_LastBonfireID)
		{
			Player->SetActorLocation(iter->GetActorLocation());
			Player->SetActorRotation(iter->GetActorRotation());
			break;
		}
	}

	applyPlayerData(Player);
}
