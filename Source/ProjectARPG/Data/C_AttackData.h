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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	E_AttackProperty eProperty;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float fDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float fPostureDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bUnblockable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanParry;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float fGuardPushBack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float fRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float fMinRange = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float fIdealRange = 220.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* pMontage;
};
