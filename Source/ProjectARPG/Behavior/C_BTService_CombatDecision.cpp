// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTService_CombatDecision.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"

void UC_BTService_CombatDecision::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AICon = OwnerComp.GetAIOwner();

	if (!AICon)
		return;

	AC_EnemyCharacter* pEnemy = Cast<AC_EnemyCharacter>(AICon->GetPawn());
	if (!pEnemy)
		return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	if (!BB)
		return;

	if (!pEnemy->canDecideAction())
		return;

	if (BB->GetValueAsEnum(AC_EnemyController::AIActionKey) !=
		static_cast<uint8>(E_EnemyCombatAction::None))
		return;

	const FS_EnemyCombatProfile& profile = pEnemy->getCombatProfile();

	float fRan = FMath::FRand();

	E_EnemyCombatAction NextAction = E_EnemyCombatAction::None;

	if (fRan <= profile.fAttackProbability)
	{
		NextAction = E_EnemyCombatAction::Attack;
	}
	else if (fRan <=
		profile.fAttackProbability + profile.fGuardProbability)
	{
		NextAction = E_EnemyCombatAction::Guard;
	}

	BB->SetValueAsEnum(
		AC_EnemyController::AIActionKey,
		static_cast<uint8>(NextAction)
	);

	
}
