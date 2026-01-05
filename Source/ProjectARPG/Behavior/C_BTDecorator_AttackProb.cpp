// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTDecorator_AttackProb.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectARPG/AI/C_EnemyController.h"

UC_BTDecorator_AttackProb::UC_BTDecorator_AttackProb()
{
	NodeName = TEXT("Random <= AttackProb");
}

bool UC_BTDecorator_AttackProb::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return false;

	const float fAttackProb = BB->GetValueAsFloat(
		AC_EnemyController::AttackProbKey
	);

	if (fAttackProb <= 0.f)
		return false;

	if (fAttackProb >= 1.f)
		return true;

	return FMath::FRand() <= fAttackProb;

}
