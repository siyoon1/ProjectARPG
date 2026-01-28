// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ProjectARPG/Interface/C_CombatDecisionStrategy.h"
#include "C_BossCombatDecision.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTARPG_API UC_BossCombatDecision : public UObject, public IC_CombatDecisionStrategy
{
	GENERATED_BODY()
public:
	virtual void Decide(AC_EnemyCharacter* Enemy,
		UBlackboardComponent* BB) override;
	
};
