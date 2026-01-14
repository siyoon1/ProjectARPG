// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "C_SaveGame.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTARPG_API UC_SaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FName m_LastBonfireID;

	UPROPERTY()
	float m_PlayerHP;

	UPROPERTY()
	float m_PlayerMaxHP;
	
};
