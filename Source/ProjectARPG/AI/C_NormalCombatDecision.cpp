// Fill out your copyright notice in the Description page of Project Settings.


#include "C_NormalCombatDecision.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectARPG/AI/C_AIAttackComponent.h"

void UC_NormalCombatDecision::Decide(AC_EnemyCharacter* Enemy, UBlackboardComponent* BB)
{
    if (!Enemy || !BB)
        return;

    if (Enemy->isCounterWindowOpen() &&
        Enemy->canDecideAction())
    {
        BB->SetValueAsEnum(
            AC_EnemyController::IntentKey,
            (uint8)E_CombatIntent::Attack);
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

    const float Dist =
        BB->GetValueAsFloat(AC_EnemyController::DistKey);

    const FS_EnemyCombatProfile& Profile =
        Enemy->getCombatProfile();

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

	const int32 AttackChain = Enemy->getPlayerAttackChain();

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
		GuardW *= 0.f;
		AttackW *= 1.4f;
	}

	if (Enemy->isGuard())
	{
		if (Enemy->canReleaseGuard())
		{
			Enemy->endGuard();

			BB->SetValueAsEnum(
				AC_EnemyController::IntentKey,
				(uint8)E_CombatIntent::None);
			return;
		}
		else
		{
			BB->SetValueAsEnum(
				AC_EnemyController::IntentKey,
				(uint8)E_CombatIntent::None);
			return;
		}
	}

	if (UC_AIAttackComponent* AIAtk =
		Enemy->getEnemyAttackComponent())
	{
		// 충분히 가까울 때만
		if (Dist < Enemy->getCombatProfile().fPreferredRange * 0.6f)
		{
			Enemy->playStepBack();

			BB->SetValueAsEnum(
				AC_EnemyController::IntentKey,
				(uint8)E_CombatIntent::None);
			return;
		}
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

	if (Dist < Profile.fPreferredRange &&
		Enemy->getPlayerAttackChain() == 0)
	{
		BB->SetValueAsEnum(
			AC_EnemyController::IntentKey,
			(uint8)E_CombatIntent::Attack);
		return;
	}
}
