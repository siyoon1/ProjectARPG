// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C_CombatCharacter.h"
#include "C_PlayerCharacter.generated.h"

UENUM(BlueprintType)
enum class E_PlayerActionState : uint8
{
	Idle,
	Sprinting,
	Dodging,
	Attacking,
};

/**
 * 
 */
UCLASS()
class PROJECTARPG_API AC_PlayerCharacter : public AC_CombatCharacter
{
	GENERATED_BODY()

private:
	E_PlayerActionState m_eState = E_PlayerActionState::Idle;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> m_pSpringArm;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> m_pCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EnhancedInput", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputMappingContext> m_pMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EnhancedInput", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> m_pMoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EnhancedInput", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> m_pLookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EnhancedInput", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> m_pSprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EnhancedInput", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> m_pComboAttackAction;

	int32 m_nCurrentComboIndex = 0;
	int32 m_nMaxComboIndex = 5;
	bool m_bCanQueueCombo = false;
	bool m_bQueuedCombo = false;
	bool m_bCanAttackRestart = false;


public:
	AC_PlayerCharacter();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void playComboSection(int32 nComboIndex);
	void stopSprintOrDodge();
	
	

protected:
	void look(const struct FInputActionValue& sValue);
	void move(const FInputActionValue& sValue);
	void comboAttack(const FInputActionValue& sValue);
	void sprint(const struct FInputActionInstance& sInst);
	void sprintReleased(const struct FInputActionInstance& sInst);
	

public:
	void setCanCombo(bool bCanCombo);
	void tryContiuneCombo();
	void resetComboState();
	void enableComboRestart();
	void setPlayerActionState(E_PlayerActionState eNewState);
	E_PlayerActionState getPlayerActionState() const;


	
};
