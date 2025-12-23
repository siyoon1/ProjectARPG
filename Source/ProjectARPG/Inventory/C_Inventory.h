// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_ItemObject.h"
#include "C_Inventory.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

USTRUCT(BlueprintType)
struct FS_InventorySlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	UC_ItemObject* Item = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 nCount = 0;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_Inventory : public UActorComponent
{
	GENERATED_BODY()


public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 m_MaxSlotCount = 20;

	UPROPERTY(BlueprintReadOnly)
	TArray<FS_InventorySlot> m_Slots;

	UPROPERTY(BlueprintAssignable)
	FOnInventoryChanged m_onInventoryChanged;


public:	
	// Sets default values for this component's properties
	UC_Inventory();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	bool addItem(UC_ItemObject* pItem, int32 nCount);

	UFUNCTION(BlueprintCallable)
	bool removeItem(FName ItemID, int32 nCount);

	UFUNCTION(BlueprintCallable)
	TArray<FS_InventorySlot>& getSlots();
};
