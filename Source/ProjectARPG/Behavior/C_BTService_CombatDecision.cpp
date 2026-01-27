// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTService_CombatDecision.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectARPG/AI/C_AIAttackComponent.h"

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

	if (pEnemy->isCounterWindowOpen() &&
		pEnemy->canDecideAction())
	{
		BB->SetValueAsEnum(
			AC_EnemyController::IntentKey,
			(uint8)E_CombatIntent::Attack);
		return;
	}

	if (pEnemy->isGuard())
	{
		// 가드 해제 가능하면 다음 행동으로 넘김
		if (pEnemy->canReleaseGuard())
		{
			BB->SetValueAsEnum(
				AC_EnemyController::IntentKey,
				(uint8)E_CombatIntent::None);
		}

		return;
	}

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

	// 거리 기반 보정
	if (Dist < Profile.fPreferredRange * 0.8f)
	{
		AttackW *= 1.3f;
		GuardW *= 0.6f;
	}
	else if (Dist > Profile.fPreferredRange * 1.1f)
	{
		AttackW *= 0.6f;
		RepoW *= 1.4f;
	}

	const int32 AttackChain = pEnemy->getPlayerAttackChain();

	if (AttackChain >= 2)
	{
		GuardW *= 0.7f;
		RepoW *= 1.2f;
	}

	if (AttackChain >= 3)
	{
		AttackW *= 1.6f;
		GuardW *= 0.3f;
	}

	const E_CombatIntent LastIntent =
		(E_CombatIntent)BB->GetValueAsEnum(AC_EnemyController::LastIntentKey);

	if (LastIntent == E_CombatIntent::Guard)
	{
		GuardW *= 0.2f;
		AttackW *= 1.2f;
	}

	if (UC_AIAttackComponent* AIAtk =
		pEnemy->getEnemyAttackComponent())
	{
		if (!AIAtk->hasExecutableAttack(Dist))
		{
			AttackW = 0.f;
			RepoW *= 1.2f;
		}
	}

	if (Dist > Profile.fPreferredRange * 1.1f)
	{
		RepoW = 0.f;
	}

	const float Sum = AttackW + GuardW + RepoW;
	if (Sum <= KINDA_SMALL_NUMBER)
		return;

	const float Pick = FMath::FRandRange(0.f, Sum);

	E_CombatIntent Intent =
		(Pick < AttackW) ? E_CombatIntent::Attack :
		(Pick < AttackW + GuardW) ? E_CombatIntent::Guard :
		E_CombatIntent::Reposition;



	BB->SetValueAsEnum(
		AC_EnemyController::IntentKey,
		(uint8)Intent);

	BB->SetValueAsEnum(
		AC_EnemyController::LastIntentKey,
		(uint8)Intent);
	
}
