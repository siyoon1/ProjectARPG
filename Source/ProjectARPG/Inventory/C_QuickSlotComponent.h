// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_QuickSlotComponent.generated.h"

class UC_ItemObject;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuickSlotChange, UC_ItemObject*, Item, int32, Count);



USTRUCT(BlueprintType)
struct FS_QuickSlot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName ItemID;

	UPROPERTY(BlueprintReadOnly)
	int32 slotIndex = -1;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_QuickSlotComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 m_MaxQuickSlotCount = 5;


	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FS_QuickSlot> m_QuickSlots;

	class UC_Inventory* m_Inventory;

public:
	UPROPERTY(BlueprintAssignable)
	FOnQuickSlotChange m_OnQuickSlotChange;

public:	
	// Sets default values for this component's properties
	UC_QuickSlotComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	bool assignItemToQuickSlot(FName ItemID, int32 nSlotIndex);

	UFUNCTION(BlueprintCallable)
	void useQuickSlot(int32 nSlotIndex);
};
