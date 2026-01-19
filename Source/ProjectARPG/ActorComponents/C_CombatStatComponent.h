// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_CombatStatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHpChanged, float, fCurrentHp, float, fMaxHp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPostureChanged, float, fCurrentPosture, float, fMaxPosture);
DECLARE_MULTICAST_DELEGATE(FOnPostureBroken);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_CombatStatComponent : public UActorComponent
{
	GENERATED_BODY()

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
	FOnHpChanged m_OnHpChanged;
	FOnPostureChanged m_OnPostureChanged;
	FOnPostureBroken m_OnPostureBroken;

public:	
	// Sets default values for this component's properties
	UC_CombatStatComponent();

	void applyDamage(float HpDamage, float PostureDamage);

	inline float getCurrentHp() const { return m_CurrentHp; }
	inline float getCurrentPosture() const { return m_CurrentPosture; }
	inline bool isPostureBroken() const { return m_bPostureBroken; }

	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void tickPostureRecovery(float DeltaTime);
	void breakPosture();

		
};
