// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BonfireActor.h"
#include "ProjectARPG/Game/C_ARPGGameInstance.h"
#include "ProjectARPG/Character/C_PlayerCharacter.h"

void AC_BonfireActor::activateBonfire()
{
	if (m_bActivated)
		return;

	m_bActivated = true;

	// ÀÌÆåÆ® Ãß°¡ °¡´É
}

void AC_BonfireActor::setCheckPoint()
{
	if (UC_ARPGGameInstance* GI = GetGameInstance<UC_ARPGGameInstance>())
	{
		GI->setLastBonfireID(m_BonfireID);
	}
}

// Sets default values
AC_BonfireActor::AC_BonfireActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AC_BonfireActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AC_BonfireActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AC_BonfireActor::interact_Implementation(AActor* Interactor)
{
	AC_PlayerCharacter* Player = Cast<AC_PlayerCharacter>(Interactor);
	if (!Player)
		return;

	activateBonfire();
	setCheckPoint();
	healPlayer(Player);
	saveGame();

	// ÈÞ½Ä
}

FName AC_BonfireActor::getBonfireID() const
{
	return m_BonfireID;
}

void AC_BonfireActor::saveGame()
{
	if (UC_ARPGGameInstance* GI = GetGameInstance<UC_ARPGGameInstance>())
	{
		GI->saveGame();
	}
}

void AC_BonfireActor::healPlayer(AC_PlayerCharacter* Player)
{
	if (!Player)
		return;

	Player->restoreHP();
	Player->resetPosture();
}

