// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C_ExecutionTypes.generated.h"

UENUM(BlueprintType)
enum class E_ExecutionType : uint8
{
	None			UMETA(DisplayName = "None"),
	PostureBreak	UMETA(DisplayName = "PostureBreak"),
	Stealth     UMETA(DisplayName = "Stealth")
};

class PROJECTARPG_API C_ExecutionTypes
{
public:
	
};


