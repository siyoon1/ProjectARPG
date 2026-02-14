// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_CurrencyComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCurrencyChanged);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_CurrencyComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 m_Gold = 0;

public:
	UPROPERTY(BlueprintAssignable)
	FOnCurrencyChanged m_OnCurrencyChanged;

public:	
	UC_CurrencyComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UFUNCTION(BlueprintCallable)
	int32 getGold() const;

	UFUNCTION(BlueprintCallable)
	void addGold(int32 Amount);

	UFUNCTION(BlueprintCallable)
	bool spendGold(int32 Amount);

		
};
