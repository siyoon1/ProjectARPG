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

	// Ã¼°£ ºØ±« ÀÎ»ì (·£´ý)
	UPROPERTY(EditDefaultsOnly, Category = "Execution")
	TArray<TObjectPtr<UAnimMontage>> PostureBreakExecutions;

	// ÈÄ¹æ ¾Ï»ì (°íÁ¤)
	UPROPERTY(EditDefaultsOnly, Category = "Execution")
	TObjectPtr<UAnimMontage> StealthExecution;

public:
	void playHitMontage(E_Direction eDir);
	void playParryMontage(E_ParryDirection eDir);

	void playExecutionMontage(E_ExecutionType Type);
};
