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

	if (BB->GetValueAsBool(AC_EnemyController::IntentLockedKey))
		return;

	if (!pEnemy->canDecideAction())
		return;

	if (pEnemy->isExecutingAction())
		return;

	AActor* Target =
		Cast<AActor>(BB->GetValueAsObject("TargetActor"));

	if (!Target)
	{
		BB->SetValueAsEnum(
			AC_EnemyController::IntentKey,
			(uint8)E_CombatIntent::None);
		return;
	}

	const float Dist = BB->GetValueAsFloat(AC_EnemyController::DistKey);


	const FS_EnemyCombatProfile& Profile = pEnemy->getCombatProfile();

	if (Dist > Profile.fPreferredRange * 1.3f)
	{
		BB->SetValueAsEnum(
			AC_EnemyController::IntentKey,
			(uint8)E_CombatIntent::Chase);
		return;
	}


	float AttackW = Profile.fAttackProbability;
	float GuardW = Profile.fGuardProbability;
	float RepoW = 0.3f;

	const bool bCanAttack =
		pEnemy->canConsiderAttack(Dist);

	if (!bCanAttack)
	{
		AttackW = 0.f;
		GuardW *= 1.3f;
		RepoW *= 0.8f;
	}

	

	// 상황 가중치
	if (pEnemy->isPlayerAttacking())
		GuardW *= 1.5f;

	if (Dist < Profile.fPreferredRange * 0.8f)
		AttackW *= 1.2f;

	const E_CombatIntent LastIntent =
		(E_CombatIntent)BB->GetValueAsEnum(
			AC_EnemyController::LastIntentKey);

	if (LastIntent == E_CombatIntent::Reposition)
	{
		RepoW *= 0.25f;   // 연속 리포지션 강력 억제
		AttackW *= 1.2f;
	}
	else if (LastIntent == E_CombatIntent::Attack)
	{
		AttackW *= 0.85f;
		GuardW *= 1.1f;
	}

	const float Sum = AttackW + GuardW + RepoW;
	const float Pick = FMath::FRandRange(0.f, Sum);

	E_CombatIntent Intent = E_CombatIntent::Guard;

	if (Pick < AttackW)
		Intent = E_CombatIntent::Attack;
	else if (Pick < AttackW + GuardW)
		Intent = E_CombatIntent::Guard;
	else
		Intent = E_CombatIntent::Reposition;

	BB->SetValueAsEnum(
		AC_EnemyController::IntentKey,
		(uint8)Intent);

	BB->SetValueAsBool(
		AC_EnemyController::IntentLockedKey,
		true);

	BB->SetValueAsEnum(
		AC_EnemyController::LastIntentKey,
		(uint8)Intent);
	
}
