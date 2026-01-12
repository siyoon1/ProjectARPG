// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_ShopComponent.generated.h"

USTRUCT(BlueprintType)
struct FS_ShopItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName ItemID;

	UPROPERTY(BlueprintReadOnly)
	int32 Pirce;
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
	bool buyItem(FName ItemID, int32 nCount, class UC_Inventory* PlayerInventory);

	UFUNCTION(BlueprintCallable)
	bool sellItem(FName ItemID, int32 nCount, class UC_Inventory* PlayerInventory);
};
