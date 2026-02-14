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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxStack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	E_ItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
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

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool use(class AC_CombatCharacter* User);
	virtual bool use_Implementation(AC_CombatCharacter* User);

};
