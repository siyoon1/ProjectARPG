// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
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

	UFUNCTION()
	void AnimNotify_DodgeEnd();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
	bool m_bIsDodging;

	FDelegateHandle m_sDelegateHandle;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> m_pDodgeMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> m_pSprintStartMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> m_pComboAttackMontage;

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUninitializeAnimation() override;

	UFUNCTION()
	void onMontageEnded(UAnimMontage* Montage, bool bInterrupted);

public:
	void playDodgeMontage(E_Direction eDir);
	void playSprintStartMontage();
	void playComboMontageSection(FName strSectionName);
	
};
