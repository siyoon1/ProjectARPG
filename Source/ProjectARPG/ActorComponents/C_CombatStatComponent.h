// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_CombatStatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHpChanged, float, fCurrentHp, float, fMaxHp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPostureChanged, float, fCurrentPosture, float, fMaxPosture);
DECLARE_MULTICAST_DELEGATE(FOnPostureBroken);

UENUM()
enum class E_PostureBreakCause : uint8
{
	Unknown,
	Damage,
	Parry
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_CombatStatComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, Category = "Stat")
	UDataTable* m_StatTable;

	UPROPERTY(EditDefaultsOnly, Category = "Stat")
	FName m_StatRowName;

	E_PostureBreakCause m_LastBreakCause = E_PostureBreakCause::Unknown;
	AActor* m_LastBreaker;

private:
	UPROPERTY(EditAnywhere)
	float m_MaxHp = 100.f;

	float m_CurrentHp;

	UPROPERTY(EditAnywhere)
	float m_MaxPosture = 100.f;

	float m_CurrentPosture;

	UPROPERTY(EditAnywhere)
	float m_PostureRecoveryRate = 20.f;

	UPROPERTY(EditAnywhere)
	float m_RecoveryDelayTime = 1.0f;

	float m_RecoveryDelayTimer = 0.f;

	bool m_bRecoveryDelayed = false;
	bool m_bPostureBroken = false;

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnHpChanged m_OnHpChanged;
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnPostureChanged m_OnPostureChanged;

	FOnPostureBroken m_OnPostureBroken;

public:	
	// Sets default values for this component's properties
	UC_CombatStatComponent();

	void applyDamage(float HpDamage, float PostureDamage);

	void applyPostureDamage(float PostureDamage, E_PostureBreakCause Cause, AActor* Instigator);

	UFUNCTION(BlueprintCallable)
	inline float getCurrentHp() const { return m_CurrentHp; }
	UFUNCTION(BlueprintCallable)
	inline float getMaxHp() const { return m_MaxHp; }
	UFUNCTION(BlueprintCallable)
	inline float getCurrentPosture() const { return m_CurrentPosture; }
	UFUNCTION(BlueprintCallable)
	inline float getMaxPosture() const { return m_MaxPosture; }

	UFUNCTION(BlueprintCallable)
	void recoverHp(float Hp);

	UFUNCTION(BlueprintCallable)
	void recoverPosture(float Posture);

	inline bool isPostureBroken() const { return m_bPostureBroken; }

	inline const E_PostureBreakCause getBreakCause() const { return m_LastBreakCause; }
	inline AActor* getLastBreak() const { return m_LastBreaker; }

	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void initStat();

private:
	
	void tickPostureRecovery(float DeltaTime);
	void breakPosture();

		
};
