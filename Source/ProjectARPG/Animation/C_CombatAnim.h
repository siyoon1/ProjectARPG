// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ProjectARPG/Character/C_CombatCharacter.h"
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

public:
	void playHitMontage(E_Direction eDir);
};
