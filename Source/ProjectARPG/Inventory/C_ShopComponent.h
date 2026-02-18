// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_ShopComponent.generated.h"


class UC_ItemObject;

USTRUCT(BlueprintType)
struct FS_ShopItem
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName ItemID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Price;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Count;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_ShopComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FS_ShopItem> m_ItemsForSale;

public:	
	// Sets default values for this component's properties
	UC_ShopComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	const FS_ShopItem* findShopItem(FName ItemID) const;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	bool buyItem(FName ItemID, int32 nCount, class UC_Inventory* PlayerInventory, class UC_CurrencyComponent* CurrencyComp);

	UFUNCTION(BlueprintCallable)
	bool sellItem(FName ItemID, int32 nCount, class UC_Inventory* PlayerInventory, class UC_CurrencyComponent* CurrencyComp);

	UFUNCTION(BlueprintCallable)
	const TArray<FS_ShopItem>& getShopItems() const { return m_ItemsForSale; }

	UFUNCTION(BlueprintCallable)
	int32 subItemCount(int32 Index, int32 Count);
};
