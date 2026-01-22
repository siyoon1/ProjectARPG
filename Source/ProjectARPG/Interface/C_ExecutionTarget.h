// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ProjectARPG/Enums/C_ExecutionTypes.h"
#include "C_ExecutionTarget.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UC_ExecutionTarget : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTARPG_API IC_ExecutionTarget
{
	GENERATED_BODY()
public:
	// 처형 가능 여부 (Posture / Stealth 포함)
	virtual bool canBeExecuted(E_ExecutionType Type) const = 0;

	// ExecutionComponent가 호출
	virtual void onExecutionStarted(APawn* ExecutionInstigator, const FS_ExecutionContext& Context) = 0;

	// 애니메이션 끝나면 호출
	virtual void onExecutionFinished(APawn* ExecutionInstigator) = 0;

	// UI / VFX
	virtual void setExecutionHintVisible(bool bVisible) = 0;
	
};
