// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ProjectARPG/Inventory/C_ItemDataBase.h"
#include "C_ARPGGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTARPG_API UC_ARPGGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Database")
	TSubclassOf<UC_ItemDataBase> m_ItemDBClass;


	UPROPERTY()
	UC_ItemDataBase* m_ItemDB;

	UPROPERTY()
	FName m_LastBonfireID;

	UPROPERTY()
	class UC_SaveGame* m_SaveGame;

private:
	void applyPlayerData(class AC_PlayerCharacter* Player);

public:
	virtual void Init() override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UC_ItemDataBase* getItemDB() const { return m_ItemDB; }

	void setLastBonfireID(FName NewID);
	FName getLastBonfireID() const;

	void saveGame();
	void loadGame();

	
	void collectPlayerData(AC_PlayerCharacter* Player);
	void respawnPlayer(AC_PlayerCharacter* Player);
};
