// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "C_ItemObject.generated.h"

UENUM(BlueprintType)
enum class E_ItemType : uint8
{
	Consumable,
	Equipment
};

USTRUCT(BlueprintType)
struct FS_ItemData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FName ItemID;

	UPROPERTY(EditAnywhere)
	FText ItemName;

	UPROPERTY(EditAnywhere)
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere)
	int32 MaxStack;

	UPROPERTY(EditAnywhere)
	E_ItemType ItemType;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UC_ItemObject> ItemClass;
};

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, Abstract)
class PROJECTARPG_API UC_ItemObject : public UObject
{
	GENERATED_BODY()

public:
	const FS_ItemData* m_ItemData;
	

public:
	void init(const FS_ItemData* InData);

	UFUNCTION(BlueprintCallable)
	FText getItemName() const;

	UFUNCTION(BlueprintCallable)
	UTexture2D* getItemIcon() const;

	UFUNCTION(BlueprintCallable)
	FName getItemID() const;

};
