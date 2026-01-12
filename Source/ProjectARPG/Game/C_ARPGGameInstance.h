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
	UC_ItemDataBase* ItemDB;

public:
	virtual void Init() override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UC_ItemDataBase* getItemDB() const { return ItemDB; }


};
