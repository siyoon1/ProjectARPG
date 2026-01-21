// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ProjectARPG/Character/C_CombatCharacter.h"
#include "ProjectARPG/Enums/C_ExecutionTypes.h"
#include "C_CombatAnim.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTARPG_API UC_CombatAnim : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> m_pHitLeftMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> m_pHitRightMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> m_pHitBackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> m_DefaultParryMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> m_RightParryMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> m_LeftParryMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Execution")
	TArray<TObjectPtr<UAnimMontage>> m_PostureBreakExecutions;

	UPROPERTY(EditDefaultsOnly, Category = "Execution")
	TArray<TObjectPtr<UAnimMontage>> m_StealthExecutions;

public:
	void playHitMontage(E_Direction eDir);
	void playParryMontage(E_ParryDirection eDir);

	void playExecutionMontage(UAnimMontage* Montage);
};
