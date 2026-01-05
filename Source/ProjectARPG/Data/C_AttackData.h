// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "C_AttackData.generated.h"

UENUM(BlueprintType)
enum class E_AttackProperty : uint8
{
	Normal,
	Heavy,
	Thrust,
	Sweep,
	GuardBreak
};
/**
 * 
 */
USTRUCT(BlueprintType)
struct FS_AttackData : public FTableRowBase
{
	GENERATED_BODY()

};
