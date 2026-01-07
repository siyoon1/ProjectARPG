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
USTRUCT()
struct FS_AttackRuntimeState
{
	GENERATED_BODY()

	float LastUsedTime = -9999.f;
	float Cooldown = 0.f;
	float Fatigue = 0.f;
};

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
	float fMaxRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float fMinRange = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float fIdealRange = 220.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float fBaseWeight = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float fRepeatPenalty = 0.6f; // 연속 사용 시 가중치 감소

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float fFatigueRecovery = 0.3f; // 시간당 회복량

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float fMinReuseTime = 0.f; // 0이면 연속 사용 가능

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* pMontage;
};
