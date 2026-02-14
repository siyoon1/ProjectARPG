// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "C_ItemDataBase.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTARPG_API UC_ItemDataBase : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	UDataTable* m_ItemDataTable;

public:
	const struct FS_ItemData* findItemData(FName ItemID) const;

	UFUNCTION(BlueprintCallable)
	bool findItemData_BP(FName ItemID, FS_ItemData& OutData) const;

	UFUNCTION(BlueprintCallable)
	UC_ItemObject* createItemObject(FName ItemID, UObject* Owner);
	
};
