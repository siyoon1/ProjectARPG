// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectARPG/Data/C_ExecutionDataBase.h"
#include "C_PlayerExecutionData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class PROJECTARPG_API UC_PlayerExecutionData : public UC_ExecutionDataBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<E_ExecutionType, FS_ExecutionMontageArray> m_ExecutionMontages;

public:
	virtual UAnimMontage* selectMontage(E_ExecutionType Type, int32 Index) const override
	{
		const FS_ExecutionMontageArray* Entry = m_ExecutionMontages.Find(Type);

		if (!Entry || Entry->Montages.Num() == 0)
			return nullptr;

		return Entry->Montages[Index % Entry->Montages.Num()];
	}
	
};
