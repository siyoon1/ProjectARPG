// Fill out your copyright notice in the Description page of Project Settings.


#include "C_NormalCombatDecision.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "ProjectARPG/AI/C_AIAttackComponent.h"

void UC_NormalCombatDecision::calculateScores(AC_EnemyCharacter* Enemy, UBlackboardComponent* BB)
{
    if (!Enemy || !BB)
        return;

    if (!Enemy->canDecideAction())
        return;

    const float Dist = BB->GetValueAsFloat(AC_EnemyController::DistKey);
    const FS_EnemyCombatTendency& Tendency = Enemy->getCombatTendency();

    float AttackScore = 0.f;
    float GuardScore = 0.f;
    float RepositionScore = 0.f;
    float ChaseScore = 0.f;

    const float SafeRange = FMath::Max(Tendency.PreferredRange, 1.f);
    const float ChaseStart = SafeRange * 1.3f;
    const float RepositionStart = SafeRange * 0.55f;

    UC_AIAttackComponent* AttackComp = Enemy->getEnemyAttackComponent();
    const bool bCanAttack = (AttackComp && AttackComp->hasExecutableAttack(Dist));

    // -----------------------------
    // 1?? Chase (거리 멀면 무조건 추격)
    // -----------------------------
    if (Dist > ChaseStart)
    {
        ChaseScore = 2.0f;
        AttackScore = 0.f;
        GuardScore *= 0.5f;
    }
    else
    {
        // -----------------------------
        // 2?? Attack
        // -----------------------------
        if (bCanAttack)
        {
            // 거리 기반 공격 우선순위
            float RangeFactor = 1.f - FMath::Clamp(FMath::Abs(Dist - SafeRange) / SafeRange, 0.f, 1.f);
            AttackScore = Tendency.Aggressiveness * 0.7f + RangeFactor * 0.3f;

            // 최근 공격 감쇠
            if (Enemy->getLastAction() == E_EnemyActionType::Attack)
            {
                AttackScore *= 0.85f;
            }
        }
        else
        {
            AttackScore = 0.f;
        }

        if (!bCanAttack && Dist < SafeRange * 0.6f)
        {
            // 공격 후보 없어도 백스텝 선택이 자연스럽도록 RepositionScore 강화
            RepositionScore = FMath::Max(RepositionScore, Tendency.RepositionBias);
        }

        // -----------------------------
        // 3?? Guard
        // -----------------------------
        float ChainFactor = FMath::Clamp(Enemy->getPlayerAttackChain() / 3.f, 0.f, 1.f);
        GuardScore = Tendency.DefenseBias * (0.4f + ChainFactor * 0.8f);

        if (Enemy->getLastAction() == E_EnemyActionType::Guard)
        {
            GuardScore *= 0.7f;
        }

        // 공격 불가 시 근거리에서는 가드 감점
        if (!bCanAttack && Dist < SafeRange * 0.6f)
        {
            GuardScore *= 0.7f;
        }

        // -----------------------------
        // 4?? Reposition (너무 가까우면 백스텝)
        // -----------------------------
        if (Dist < RepositionStart)
        {
            RepositionScore = Tendency.RepositionBias * (1.f - Dist / RepositionStart);

            // 공격 불가면 보너스
            if (!bCanAttack)
            {
                RepositionScore += 0.5f;
            }
        }

        if (Enemy->getLastAction() == E_EnemyActionType::Reposition)
        {
            RepositionScore = 0.f;  // 연속 백스텝 방지
        }
    }

    // -----------------------------
    // Blackboard 저장 (Notify Observer = Both)
    // -----------------------------
    BB->SetValueAsFloat(AC_EnemyController::AttackScoreKey, AttackScore);
    BB->SetValueAsFloat(AC_EnemyController::GuardScoreKey, GuardScore);
    BB->SetValueAsFloat(AC_EnemyController::RepositionScoreKey, RepositionScore);
    BB->SetValueAsFloat(AC_EnemyController::ChaseScorekey, ChaseScore);
}

