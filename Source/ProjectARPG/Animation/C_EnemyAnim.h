// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectARPG/Animation/C_CombatAnim.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"
#include "C_EnemyAnim.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTARPG_API UC_EnemyAnim : public UC_CombatAnim
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> m_pAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> m_pLightAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> m_pHeavyAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> m_pThrustAttackMontage;

public:
	void playAttackByType(E_EnemyAttackType eType);

	void playAttackMontage();
	UFUNCTION()
	void AnimNotify_EndAttack();
};
