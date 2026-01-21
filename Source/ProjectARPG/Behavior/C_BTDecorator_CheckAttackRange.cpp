// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTDecorator_CheckAttackRange.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "ProjectARPG/ActorComponents/C_EnemyAttackComponent.h"

UC_BTDecorator_CheckAttackRange::UC_BTDecorator_CheckAttackRange()
{
	NodeName = "Check Attack Range";
}

bool UC_BTDecorator_CheckAttackRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return false;

	float Dist = BB->GetValueAsFloat(AC_EnemyController::DistKey);
	FName AttackRow = BB->GetValueAsName(AC_EnemyController::SelectAttackKey);

	if (AttackRow.IsNone())
		return false;

	AAIController* AICon = OwnerComp.GetAIOwner();
	AC_EnemyCharacter* Enemy =
		Cast<AC_EnemyCharacter>(AICon->GetPawn());

	if (!Enemy)
		return false;

	const FS_AttackData* Data = Enemy->getAttackData(AttackRow);
	if (!Data)
		return false;

	return Enemy->getAttackComponent()->isAttackInRange(*Data, Dist);
}
