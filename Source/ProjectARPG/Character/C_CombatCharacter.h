// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C_BaseCharacter.h"
#include "ProjectARPG/Interface/C_CombatInterface.h"
#include "C_CombatCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHpChanged, float, fCurrentHp, float, fMaxHp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPostureChanged, float, fCurrentPosture, float, fMaxPosture);

UENUM(BlueprintType)
enum class E_CombatState : uint8
{
	Idle,
	Sprinting,
	Dodging,
	Attacking,
	Executing,
	Guard,
	Climb,
	Die
};

UENUM(BlueprintType)
enum class E_AttackType : uint8
{
	Normal,
	Air,
	Charge
};
/**
 * 
 */
UCLASS(Blueprintable)
class PROJECTARPG_API AC_CombatCharacter : public AC_BaseCharacter, public IC_CombatInterface
{
	GENERATED_BODY()

protected:
	E_CombatState m_eState = E_CombatState::Idle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UDataTable* m_pPostureStatsTable{};

	struct FS_PostureStats* m_sPostureStats{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Posture", meta = (AllowPrivateAccess = "true"))
	FName m_sPostureRowName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Posture", meta = (AllowPrivateAccess = "true"))
	float m_fCurrentPosture = 0.f;

	float m_fMaxPosture = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hp", meta = (AllowPrivateAccess = "true"))
	float m_fMaxHp = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hp", meta = (AllowPrivateAccess = "true"))
	float m_fCurrentHp = 0.f;

	float m_fRecoveryRate = 0.f;
	float m_fRecoveryDelayTimer = 0.f;
	float m_fBrokenDuration = 0.f;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "trace", meta = (AllowPrivateAccess = "true"))
	float m_fTraceRadius = 40.f;

	bool m_bIsTracing = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "trace", meta = (AllowPrivateAccess = "true"))
	float m_fAttackDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "trace", meta = (AllowPrivateAccess = "true"))
	float m_fPostureDamage = 0.f;

	FVector m_vLastTraceStart{};
	FVector m_vLastTraceEnd{};
	TArray<AActor*> m_HitActors{};

	E_AttackType m_eAttackType = E_AttackType::Normal;

protected:
	bool m_bIsPostureBroken = false;
	bool m_bIsRecoveryDelay = false;
	bool m_bWasParried = false;
	
	FTimerHandle m_timerHandle_PostureBroken;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatTrace")
	TObjectPtr<USceneComponent> m_pTraceStart;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatTrace")
	TObjectPtr<USceneComponent> m_pTraceEnd;

	UPROPERTY(BlueprintAssignable, Category = "Status")
	FOnHpChanged m_OnHpChanged;

	UPROPERTY(BlueprintAssignable, Category = "Status")
	FOnPostureChanged m_OnPostureChanged;

	UPROPERTY()
	TObjectPtr<class UC_ExecutionComponent> m_pExecutionCom;

	UPROPERTY()
	TObjectPtr<class UC_ParryComponent> m_pParryCom;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void onPostureBroken();


public:
	AC_CombatCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void setCombatState(E_CombatState eNewState);
	E_CombatState getCombatState() const;

	bool isGuardingFront(AActor* pAttacker) const;

	UFUNCTION(BlueprintCallable)
	void setHp(float fHp);
	UFUNCTION(BlueprintCallable)
	float getHp() const;
	UFUNCTION(BlueprintCallable)
	float getMaxHp() const;
	UFUNCTION(BlueprintCallable)
	float getPosture() const;
	UFUNCTION(BlueprintCallable)
	float getMaxPosture() const;

	UFUNCTION()
	void startAttackTrace();
	UFUNCTION()
	void stopAttackTrace();
	UFUNCTION()
	void performAttackTrace();

	UFUNCTION()
	void takeDamage_Implementation(float fDamage, float fPostureDamage);

	UFUNCTION()
	FVector getLocation_Implementation();

	UFUNCTION()
	void tryParry_Implementation(AActor* ParryOwner);

	UFUNCTION()
	void onParrySuccess_Implementation(AActor* ParryTarget);
	
};
