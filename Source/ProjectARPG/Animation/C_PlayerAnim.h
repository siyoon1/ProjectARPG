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

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> m_pDodgeMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> m_pSprintStartMontage;

public:
	void playDodgeMontage(E_Direction eDir);
	void playSprintStartMontage();
	
};
