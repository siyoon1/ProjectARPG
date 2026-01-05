// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTDecorator_IsInCombat.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "ProjectARPG/AI/C_EnemyController.h"

UC_BTDecorator_IsInCombat::UC_BTDecorator_IsInCombat()
{
	NodeName = TEXT("Is In Combat");
}

bool UC_BTDecorator_IsInCombat::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	if (!BB)
		return false;


	return BB->GetValueAsBool(AC_EnemyController::IsCombatKey);
}
