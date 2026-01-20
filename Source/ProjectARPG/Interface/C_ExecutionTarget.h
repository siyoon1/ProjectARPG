// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
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
	virtual bool canBeExecuted() const = 0;
	virtual void onExecutionStarted() = 0;
	virtual void onExecuted() = 0;

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
};
