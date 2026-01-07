// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTDecorator_CheckActionState.h"
#include "AIController.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"

UC_BTDecorator_CheckActionState::UC_BTDecorator_CheckActionState()
{
	NodeName = "Check Action State";
}

bool UC_BTDecorator_CheckActionState::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
		return false;

	AC_EnemyCharacter* Enemy = Cast<AC_EnemyCharacter>(AICon->GetPawn());
	if (!Enemy)
		return false;

	return Enemy->canDecideAction();
}
