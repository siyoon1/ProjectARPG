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

USTRUCT(BlueprintType)
struct FS_ExecutionContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	E_ExecutionType Type;

	UPROPERTY(BlueprintReadOnly)
	int32 Index;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Instigator;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Victim;

	UPROPERTY(BlueprintReadOnly)
	FVector InstigatorForward;
};

class PROJECTARPG_API C_ExecutionTypes
{
public:
	
};


