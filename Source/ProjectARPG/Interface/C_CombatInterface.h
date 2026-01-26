// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ProjectARPG/Data/C_AttackData.h"
#include "C_CombatInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UC_CombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTARPG_API IC_CombatInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void takeDamage(float Damage, float PostureDamage, E_HitResult HitResult, AActor* pAttacker);

};

