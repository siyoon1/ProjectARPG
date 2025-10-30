// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C_CombatCharacter.h"
#include "C_PlayerCharacter.generated.h"


/**
 * 
 */
UCLASS(Blueprintable)
class PROJECTARPG_API AC_PlayerCharacter : public AC_CombatCharacter
{
	GENERATED_BODY()

private:
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EnhancedInput", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> m_pJumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EnhancedInput", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> m_pGuardAction;

	UPROPERTY()
	class USphereComponent* m_pExecutionDetectSphere;


	int32 m_nCurrentComboIndex = 0;
	int32 m_nMaxComboIndex = 5;
	bool m_bNextComboQueued = false;
	float m_fLastAttackInputTime = 0.f;
	float m_fInputBuffer = 0.25f;
	float m_fDefaultSpeed = 800.f;


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
	void playCombo(int32 nComboIndex);
	
	

protected:
	void look(const struct FInputActionValue& sValue);
	void move(const FInputActionValue& sValue);
	void comboAttack(const FInputActionValue& sValue);
	void sprint(const struct FInputActionInstance& sInst);
	void sprintReleased(const struct FInputActionInstance& sInst);
	void jumpStart(const FInputActionValue& sValue);
	void jumpEnd(const FInputActionValue& sValue);
	void guard(const FInputActionInstance& sInst);
	void guardEnd(const FInputActionValue& sValue);
	

public:
	void setCombatState(E_CombatState eNewState) override;
	void onComboTransition();
	void resetCombo();
	bool tryExcuteEnemy();

	
};
