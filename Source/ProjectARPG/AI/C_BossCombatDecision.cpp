// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BossCombatDecision.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"

void UC_BossCombatDecision::Decide(AC_EnemyCharacter* Enemy, UBlackboardComponent* BB)
{
	if (!Enemy || !BB)
		return;
}
