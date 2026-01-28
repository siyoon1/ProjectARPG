// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "C_AttackData.generated.h"

UENUM()
enum class E_HitResult : uint8
{
	Normal,
	Guarded,
	Parried,
	PostureBroken,
	Execution
};

UENUM(BlueprintType)
enum class E_AttackProperty : uint8
{
	Normal,
	Heavy,
	Thrust,
	Sweep,
	GuardBreak
};



UENUM(BlueprintType)
enum class E_ParryDirection : uint8
{
	None,
	Left,
	Right,
	Both
};

USTRUCT()
struct FS_AttackRuntimeState
{
	GENERATED_BODY()

	float LastUsedTime = -9999.f;
	float Cooldown = 0.f;
	float Fatigue = 0.f;
};

USTRUCT(BlueprintType)
struct FS_AttackCombatData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float Damage = 0.f;

	UPROPERTY(EditAnywhere)
	float PostureDamage = 0.f;

	UPROPERTY(EditAnywhere)
	bool bUnblockable = false;

	UPROPERTY(EditAnywhere)
	bool bCanParry = true;

	UPROPERTY(EditAnywhere)
	E_ParryDirection ParryDirection;

	UPROPERTY(EditAnywhere)
	float GuardPushBack = 0.f;

	UPROPERTY(EditAnywhere)
	E_AttackProperty Property;
};

USTRUCT(BlueprintType)
struct FS_AttackAIData
{
	GENERATED_BODY()

	// 공격 가능 거리
	UPROPERTY(EditAnywhere)
	float MinRange = 0.f;

	UPROPERTY(EditAnywhere)
	float MaxRange = 250.f;

	UPROPERTY(EditAnywhere)
	float IdealRange = 150.f;

	UPROPERTY(EditAnywhere)
	float BaseWeight = 1.f;

	UPROPERTY(EditAnywhere)
	float MinReuseTime = 0.f;
};

USTRUCT(BlueprintType)
struct FS_AttackAnimData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	UAnimMontage* Montage = nullptr;
};



USTRUCT(BlueprintType)
struct FS_AttackData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FS_AttackCombatData Combat;

	UPROPERTY(EditAnywhere)
	FS_AttackAIData AI;

	UPROPERTY(EditAnywhere)
	FS_AttackAnimData Anim;
};
