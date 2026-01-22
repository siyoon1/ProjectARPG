// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ProjectARPG/Enums/C_ExecutionTypes.h"
#include "C_ExecutionDataBase.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FS_ExecutionMontageArray
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<UAnimMontage*> Montages;
};

UCLASS(Abstract, BlueprintType)
class PROJECTARPG_API UC_ExecutionDataBase : public UDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	virtual UAnimMontage* selectMontage(E_ExecutionType Type, int32 Index) const
		PURE_VIRTUAL(UC_ExecutionDataBase::selectMontage, return nullptr;);
	
};
