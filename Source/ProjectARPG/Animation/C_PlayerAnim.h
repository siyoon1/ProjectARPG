// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ProjectARPG/Character/C_CombatCharacter.h"
#include "C_PlayerAnim.generated.h"

UENUM(BlueprintType)
enum class E_Direction : uint8
{
	Forward     UMETA(DisplayName = "Forward"),
	Backward    UMETA(DisplayName = "Backward"),
	Left        UMETA(DisplayName = "Left"),
	Right       UMETA(DisplayName = "Right")
};

/**
 * 
 */
UCLASS()
class PROJECTARPG_API UC_PlayerAnim : public UAnimInstance
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
	/// <summary>
	/// //////////////////
	/// </summary>
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	TArray<UAnimMontage*> m_pComboAttackMontages;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Climb")
	TObjectPtr<UAnimMontage> m_pUpToClimbMontage;

public:
	void playDodgeMontage(E_Direction eDir);
	void playSprintStartMontage();
	void playComboMontage(E_AttackType eType, int32 nComboIndex);
	void playUpToClimb();

	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void setIsGuarding(bool bNewGuard);
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	bool getIsGuarding() const;
};
