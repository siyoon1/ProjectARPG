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

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, Abstract)
class PROJECTARPG_API UC_ItemObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName m_ItemID;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText m_ItemName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UTexture2D* m_Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 m_MaxStack;


public:
	UFUNCTION(BlueprintCallable)
	FText getItemName() const;

	UFUNCTION(BlueprintCallable)
	UTexture2D* getItemIcon() const;



};
