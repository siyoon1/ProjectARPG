// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectARPG/Animation/C_CombatAnim.h"
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

public:
	void playAttackMontage();
	UFUNCTION()
	void AnimNotify_EndAttack();
};
