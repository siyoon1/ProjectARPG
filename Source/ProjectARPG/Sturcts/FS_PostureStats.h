// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FS_PostureStats.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct PROJECTARPG_API FS_PostureStats : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	float fMaxHp;

	//최대 체간 수치
	UPROPERTY(EditAnywhere)
	float fMaxPosture = 100.f;

	// 회복 속도 (초당)
	UPROPERTY(EditAnywhere)
	float fRecoveryRate = 10.f;

	UPROPERTY(EditAnywhere)
	float fRecoveryDelay = 0.f;

	UPROPERTY(EditAnywhere)
	float fBrokenDuration = 0.f;
};
