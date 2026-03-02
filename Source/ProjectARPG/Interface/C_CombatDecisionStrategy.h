// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "C_CombatDecisionStrategy.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UC_CombatDecisionStrategy : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTARPG_API IC_CombatDecisionStrategy
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void calculateScores(class AC_EnemyCharacter* Enemy,
		class UBlackboardComponent* BB) = 0;
};
