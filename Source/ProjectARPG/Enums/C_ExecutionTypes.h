// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C_ExecutionTypes.generated.h"

UENUM(BlueprintType)
enum class E_ExecutionType : uint8
{
	None			UMETA(DisplayName = "None"),
	PostureBreak	UMETA(DisplayName = "PostureBreak"),
	Stealth			UMETA(DisplayName = "Stealth")
};

enum class E_ExecutionID : uint8
{

};

USTRUCT()
struct FS_ExecutionSelection
{
	GENERATED_BODY()

	FName ExecutionID;

	int32 VariantIndex;
};

class PROJECTARPG_API C_ExecutionTypes
{
public:
	
};


