// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C_BaseCharacter.h"
#include "ProjectARPG/Interface/C_CombatInterface.h"
#include "ProjectARPG/Interface/C_ParryReaction.h"
#include "ProjectARPG/Data/C_AttackData.h"
#include "C_CombatCharacter.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLifeNodeChanged, int32, nCurrentLifeNode, int32, nMaxLifeNode);


UENUM(BlueprintType)
enum class E_ActionState : uint8
{
	Free,
	Locked,
	Stunned,
	Dead
};

UENUM(BlueprintType)
enum class E_CombatMode : uint8
{
	None,
	Attacking,
	Guarding,
	Parrying,
	Executing
};

UENUM(BlueprintType)
enum class E_CombatState : uint8
{
	Idle,
	Sprinting,
	Dodging,
	Attacking,
	Executing,
	Parrying,
	Guard,
	WallGrabbing,
	Climb,
	Crouch,
	Die
};

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
UCLASS(Blueprintable)
class PROJECTARPG_API AC_CombatCharacter : public AC_BaseCharacter, public IC_CombatInterface, public IC_ParryReaction
{
	GENERATED_BODY()


protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UC_CombatStatComponent> m_StatComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UC_AttackComponent> m_AttackComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UC_ParryComponent> m_ParryCom;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ParryVFX", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UNiagaraSystem> m_ParryVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ParryVFX", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USoundBase> m_ParrySound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	E_ActionState m_ActionState = E_ActionState::Free;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	E_CombatMode m_CombatMode = E_CombatMode::None;

	// 현재 상태
	E_CombatState m_eState = E_CombatState::Idle;

	UPROPERTY(EditDefaultsOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	USoundBase* m_HitSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	USoundBase* m_GuardSound;

	// 공격 데이터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	UDataTable* m_pAttackDataTable{};

	const FS_AttackData* m_pCurrentAttackData = nullptr;

	UPROPERTY()
	FName m_CurrentAttackRow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "trace", meta = (AllowPrivateAccess = "true"))
	float m_fTraceRadius = 40.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "trace", meta = (AllowPrivateAccess = "true"))
	float m_fAttackDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "trace", meta = (AllowPrivateAccess = "true"))
	float m_fPostureDamage = 0.f;

	bool m_bCurrentAttackUnblockable = false;
	bool m_bCurrentAttackCanParry = false;
	float m_fGuardPushBack = 0.f;
	float m_fGuardStartTime = 0.f;

	// 생명력 점 (보스용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 m_MaxLifeNodes = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 m_CurrentLifeNodes = 1;

	TArray<AActor*> m_HitActors{};

	bool m_bIsPostureBroken = false;
	bool m_bIsGuarding = false;
	
	FTimerHandle m_timerHandle_PostureBroken;
	FTimerHandle m_ParriedTimer;

	//히트 스탑 관련
	UAnimMontage* m_lastMontage = nullptr;
	float m_fOriginalPlayRate = 1.f;
	FTimerHandle m_hitStopTimerHandle;
	bool m_bHitStopActive = false;

	E_ParryDirection m_RuntimeParryDir = E_ParryDirection::None;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatTrace")
	TObjectPtr<USceneComponent> m_pTraceStart;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatTrace")
	TObjectPtr<USceneComponent> m_pTraceEnd;

	UPROPERTY(BlueprintAssignable, Category = "Status")
	FOnLifeNodeChanged m_OnLifeNodeChanged;	

	UPROPERTY()
	TObjectPtr<class AC_PlayerCameraManager> m_CamMgr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void onPostureBroken();

	virtual void onPostureBroken_Internal();

	void playHitMontage(E_Direction eDir);

	E_Direction getHitDirection(AActor* pAttacker);

	UFUNCTION()
	void applyHitStop(float fSlowlate, float fDuration);

	void endHitStop();

	

	
public:
	AC_CombatCharacter();

	virtual void Tick(float DeltaTime) override;

	inline UC_CombatStatComponent* getStatComp() const { return m_StatComp; }

	// 상태

	bool canAct() const;
	void setActionState(E_ActionState eNewState);
	E_ActionState getActionState() const;

	virtual void setCombatState(E_CombatState eNewState);
	E_CombatState getCombatState() const;

	FVector getTraceStartLocation() const;
	FVector getTraceEndLocation() const;

	void enterCombatMode(E_CombatMode NewMode, E_ActionState NewActionState);


	// 공격

	void startAttack(const FS_AttackData& AttackData);
	void endAttack();

	void setCurrentAttackRow(FName RowName);
	FName getCurrentAttackRow() const;

	const UDataTable* getAttackDataTable() const;

	void applyAttack(const FS_AttackData& sData);
	const FS_AttackData* getAttackData(FName RowName) const;
	const FS_AttackData* getCurrentAttackData() const;

	void onHitConfirmed(E_HitResult Result, AActor* Attacker);
	void applyHitPushBack(const FVector& From, E_HitResult Result);


	// 가드
	virtual bool startGuard();
	virtual void endGuard();

	bool isGuardingFront(AActor* pAttacker) const;
	void setGuard(bool bSet);
	bool isGuard() const;

	virtual float getGuardPostureMultiplier() const;

	

	virtual bool isInvincibleAgainst(AActor* pAttacker) const;

	// 죽음
	UFUNCTION()
	virtual void onDeath();

	UFUNCTION(BlueprintCallable)
	bool isDead() const;

	// 패링
	UFUNCTION()
	void endParried();

	void onParried_Implementation(AActor* ParryOwner);

	UC_ParryComponent* getParryComponent() const;

	UFUNCTION()
	void onParryWindowEnded();


	UFUNCTION(BlueprintCallable)
	float getHp() const;

	UFUNCTION(BlueprintCallable)
	float getPosture() const;

	UFUNCTION()
	void takeDamage_Implementation(float Damage, float PostureDamage, E_HitResult HitResult, AActor* pAttacker);


	void setRuntimeParryDir(E_ParryDirection eDir);

	UFUNCTION()
	void onAttackParried(AC_CombatCharacter* Attacker,
		AC_CombatCharacter* Defender,
		FVector HitPoint);

private:
	void applyHitFeedback(E_HitResult HitResult, AActor* Attacker);
	void applyAttackerHitFeedback(E_HitResult HitResult, AActor* Attacker);
};
