// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTService_CombatDecision.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectARPG/AI/C_AIAttackComponent.h"
#include "ProjectARPG/AI/C_NormalCombatDecision.h"
#include "ProjectARPG/AI/C_BossCombatDecision.h"
#include "ProjectARPG/Interface/C_CombatDecisionStrategy.h"

void UC_BTService_CombatDecision::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UE_LOG(LogTemp, Warning, TEXT("CombatDecision Tick"));

	AAIController* AICon = OwnerComp.GetAIOwner();

	if (!AICon)
		return;

	AC_EnemyCharacter* pEnemy = Cast<AC_EnemyCharacter>(AICon->GetPawn());
	if (!pEnemy)
		return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	if (!BB)
		return;

	IC_CombatDecisionStrategy* Decision =
		pEnemy->isBoss() ?
		Cast<IC_CombatDecisionStrategy>(pEnemy->getBossCombatDecision()) :
		Cast<IC_CombatDecisionStrategy>(pEnemy->getNormalCombatDecision());

	if (Decision)
	{
		Decision->calculateScores(pEnemy, BB);
	}
	
}
