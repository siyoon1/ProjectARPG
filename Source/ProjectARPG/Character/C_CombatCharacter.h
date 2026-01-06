// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C_BaseCharacter.h"
#include "ProjectARPG/Interface/C_CombatInterface.h"
#include "ProjectARPG/Data/C_AttackData.h"
#include "C_CombatCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHpChanged, float, fCurrentHp, float, fMaxHp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPostureChanged, float, fCurrentPosture, float, fMaxPosture);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLifeNodeChanged, int32, nCurrentLifeNode, int32, nMaxLifeNode);

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
enum class E_AttackType : uint8
{
	Normal,
	Air,
	Charge
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
class PROJECTARPG_API AC_CombatCharacter : public AC_BaseCharacter, public IC_CombatInterface
{
	GENERATED_BODY()

protected:
	// 현재 상태
	E_CombatState m_eState = E_CombatState::Idle;

	// 공격 데이터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	UDataTable* m_pAttackDataTable{};

	const FS_AttackData* m_pCurrentAttackData = nullptr;

	// 체간 스텟
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

	bool m_bCurrentAttackUnblockable = false;
	bool m_bCurrentAttackCanParry = false;
	float m_fGuardPushBack = 0.f;

	// 생명력 점 (보스용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 m_MaxLifeNodes = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 m_CurrentLifeNodes = 1;

	FVector m_vLastTraceStart{};
	FVector m_vLastTraceEnd{};
	TArray<AActor*> m_HitActors{};

	E_AttackType m_eAttackType = E_AttackType::Normal;

	bool m_bIsDead = false;
	bool m_bExecutionAvailable = false;

	bool m_bIsPostureBroken = false;
	bool m_bIsRecoveryDelay = false;
	bool m_bWasParried = false;
	bool m_bIsGuarding = false;
	
	FTimerHandle m_timerHandle_PostureBroken;

	//히트 스탑 관련
	UAnimMontage* m_lastMontage = nullptr;
	float m_fOriginalPlayRate = 1.f;
	FTimerHandle m_hitStopTimerHandle;
	bool m_bHitStopActive = false;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatTrace")
	TObjectPtr<USceneComponent> m_pTraceStart;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatTrace")
	TObjectPtr<USceneComponent> m_pTraceEnd;

	UPROPERTY(BlueprintAssignable, Category = "Status")
	FOnHpChanged m_OnHpChanged;

	UPROPERTY(BlueprintAssignable, Category = "Status")
	FOnPostureChanged m_OnPostureChanged;

	UPROPERTY(BlueprintAssignable, Category = "Status")
	FOnLifeNodeChanged m_OnLifeNodeChanged;

	UPROPERTY()
	TObjectPtr<class UC_ExecutionComponent> m_pExecutionCom;

	UPROPERTY()
	TObjectPtr<class UC_ParryComponent> m_pParryCom;

	UPROPERTY()
	TObjectPtr<class AC_PlayerCameraManager> m_CamMgr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void onPostureBroken();

	void enterExecutionReady();

	void reduceHp(float fDamage);
	void reducePosture(float fDamage);

	void playHitMontage(E_Direction eDir);

	E_Direction getHitDirection(AActor* pAttacker);

	UFUNCTION()
	void applyHitStop(float fSlowlate, float fDuration);

	void endHitStop();

	
public:
	AC_CombatCharacter();

	virtual void Tick(float DeltaTime) override;

	// 상태 set, get
	virtual void setCombatState(E_CombatState eNewState);
	E_CombatState getCombatState() const;

	// 공격 데이터
	void applyAttack(const FS_AttackData& sData);
	const FS_AttackData* getAttackData(FName RowName) const;
	const FS_AttackData* getCurrentAttackData() const;

	// 가드
	bool isGuardingFront(AActor* pAttacker) const;
	void setGuard(bool bSet);
	bool isGuard() const;

	bool canAct() const;

	virtual bool isInvincibleAgainst(AActor* pAttacker) const;

	// 인살
	virtual void onExecuted();

	// 죽음
	virtual void onDeath();
	bool isDead() const;
	

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
	void takeDamage_Implementation(float fDamage, float fPostureDamage, bool bGuardSuccess, AActor* pAttacker);

	UFUNCTION()
	FVector getLocation_Implementation();

	UFUNCTION()
	void tryParry_Implementation(AActor* ParryOwner);

	UFUNCTION()
	void onParrySuccess_Implementation(AActor* ParryTarget);
	
};
