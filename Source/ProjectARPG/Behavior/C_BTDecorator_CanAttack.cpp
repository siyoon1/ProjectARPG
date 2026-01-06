// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTDecorator_CanAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectARPG/AI/C_EnemyController.h"

UC_BTDecorator_CanAttack::UC_BTDecorator_CanAttack()
{
	NodeName = TEXT("CanAttack");
}

bool UC_BTDecorator_CanAttack::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return false;

	return BB->GetValueAsBool(AC_EnemyController::CanAttackKey) == true;
}
