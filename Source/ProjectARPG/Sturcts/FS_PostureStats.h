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
	//최대 체간 수치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Posture")
	float fMaxPosture = 100.f;

	// 회복 속도 (초당)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Posture")
	float fRecoveryRate = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Posture")
	float fRecoveryDelay = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Posture")
	float fBrokenDuration = 0.f;
};
