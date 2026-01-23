// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectARPG/Animation/C_CombatAnim.h"
#include "C_PlayerAnim.generated.h"


/**
 * 
 */
UCLASS()
class PROJECTARPG_API UC_PlayerAnim : public UC_CombatAnim
{
	GENERATED_BODY()

private:
	FDelegateHandle m_sDelegateHandle;
	bool m_bIsGuarding;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> m_pDodgeMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> m_pSprintStartMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	TArray<UAnimMontage*> m_pComboAttackMontages;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climb")
	TObjectPtr<UAnimMontage> m_pUpToClimbMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parry")
	TObjectPtr<UAnimMontage> m_ParryLeftMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parry")
	TObjectPtr<UAnimMontage> m_ParryRightMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parry")
	TObjectPtr<UAnimMontage> m_ParryBothMontage;


public:
	void playDodgeMontage(E_Direction eDir);
	void playSprintStartMontage();
	void playComboMontage(int32 nComboIndex);
	void playUpToClimb();
	void playParrySuccessMontage(E_ParryDirection Direction);


	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void setIsGuarding(bool bNewGuard);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	bool getIsGuarding() const;
};
