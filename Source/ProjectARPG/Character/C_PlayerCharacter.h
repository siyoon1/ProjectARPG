// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "C_CombatCharacter.h"
#include "ProjectARPG/Interface/C_ExecutionRequester.h"
#include "C_PlayerCharacter.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class PROJECTARPG_API AC_PlayerCharacter : public AC_CombatCharacter, public IC_ExecutionRequester
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EnhancedInput", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> m_pLockOnAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EnhancedInput", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> m_pCrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EnhancedInput", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> m_pInteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EnhancedInput", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> m_pGrappleAction;

	UPROPERTY()
	class USphereComponent* m_pExecutionDetectSphere{};

	UPROPERTY()
	class UC_GrappleComponent* m_pGrappleCom{};

	UPROPERTY()
	class UC_InteractionComponent* m_pInteractCom{};

	UPROPERTY()
	class UC_MoveActionComponent* m_pMoveActionCom{};

	UPROPERTY()
	class AC_EnemyCharacter* m_pCurrentExecutionTarget = nullptr;

	//위젯 관련
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> m_GrappleWidgetClass{};

	FVector m_ClimbTarget;

	UPROPERTY()
	UUserWidget* m_GrappleWidget{};

	//플레이어 연속 공격 관련 변수
	int32 m_nCurrentComboIndex = 0;
	int32 m_nMaxComboIndex = 4;
	bool m_bNextComboQueued = false;
	float m_fLastAttackInputTime = 0.f;
	float m_fInputBuffer = 0.25f;

	//플레이어 기본 이동수치 관련 변수
	float m_fDefaultSpeed = 620.f;
	float m_fDefaultCrouched = 300.f;
	float m_fDefaultAcceleration = 4000.f;
	float m_fDefaultBraking = 4200.f;
	float m_fDefaultGravity = 1.5f;
	float m_fDefaultJumpVelocity = 560.f;
	float m_fDefaultAirControl = 0.35f;

	//플레이어 클라이밍
	FVector2D m_vCurrentMoveInput{};
	FVector m_vWallHitLocation{};

	bool m_bJumpPressed = false; // 점프가 눌리고 있는지
	int32 m_nJumpCount = 0;
	int32 m_MaxJumpCount = 2;


	//플레이어 웅크리기 관련 변수
	bool m_bIsCrouch = false;

	//플레이어 대시 관련 변수
	bool m_bSprintStarted = false;


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UC_PlayerExecutionComponent> m_ExecutionComp{};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UC_LockOnComponent> m_LockOnComp{};

public:
	UPROPERTY(EditDefaultsOnly, Category = "Execution")
	class UC_PlayerExecutionData* m_PlayerExecutionData;

	UPROPERTY(EditAnywhere)
	class UAnimMontage* m_ParrySuccessMontage;

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

	bool isInvincibleAgainst(AActor* pAttacker) const override;

private:
	//콤보 공격 실행
	void playCombo(int32 nComboIndex);

	//공격이 가능한지 체크
	bool canAttack() const;

	//공격 시작
	void startAttackCombo();

	//인살이 가능한지 체크
	bool canExecute() const;

	//전방에 적 찾기
	AActor* getCurrentEnemy();
	
	bool canGrabWallAtLoc(const FVector& checkLoc);

	//착지 상태
	void Landed(const FHitResult& Hit) override;

	//인살 시도 함수
	bool tryStartExecution();

	void applyExecutionWarp(const struct FS_ExecutionContext& Context);

	void applyLockOnRotation(float DeltaTime);
	void applyLockOnCharacterRotation(float DeltaTime);

	void releaseLockOnState();

protected:
	void sprint(const struct FInputActionInstance& sInst);
	void sprintReleased(const FInputActionInstance& sInst);
	void guard(const FInputActionInstance& sInst);
	void look(const struct FInputActionValue& sValue);
	void move(const FInputActionValue& sValue);
	void comboAttack(const FInputActionValue& sValue);
	void jumpStart(const FInputActionValue& sValue);
	void jumpEnd(const FInputActionValue& sValue);
	void guardEnd(const FInputActionValue& sValue);
	void parry(const FInputActionValue& sValue);
	void lockOn(const FInputActionValue& sValue);
	void crouch(const FInputActionValue& sValue);
	void interact(const FInputActionValue& sValue);
	void grapple(const FInputActionValue& sValue);

	FName getComboAttackRow(int32 ComboIndex) const;

public:
	void onActionFinished();
	void onExecutionFinished();


	void setCombatState(E_CombatState eNewState) override;
	E_CombatState getCombatState() const;

	float getDefaultGravity() const;
	float getDefaultAirControl() const;


	void onComboTransition();
	void resetCombo();
	USphereComponent* getExecutionSphere() const;

	void restoreHP();
	void resetPosture();


	void onParried_Implementation(AActor* ParryOwner) override;
	void onParrySuccess_Implementation(AActor* ParryTarget, E_ParryDirection Direction) override;


	void playPlayerExecutionMontage(const FS_ExecutionContext& Context);

	//락온 함수
	UFUNCTION(BlueprintCallable)
	bool isLockOn() const;

	//클라이밍 함수
	UFUNCTION(BlueprintCallable)
	bool isWallGrab() const;

	//웅크리기 함수
	UFUNCTION(BlueprintCallable)
	bool isCrouch() const;

	//벽짚기 가능확인 함수
	UFUNCTION(BlueprintCallable)
	bool isCanWallGrab() const;

	//벽짚기 함수
	UFUNCTION(BlueprintCallable)
	bool isWallGrabbing() const;

	inline void setClimbTarget(const FVector& Target) { m_ClimbTarget = Target; }
	inline const FVector& getClimbTarget() const { return m_ClimbTarget; }

	UFUNCTION(BlueprintCallable)
	bool isPulling() const;

	bool isPlayerControlled() const;

	UCameraComponent* getFollowCamera() const;

	void initJump();

	void interruptMoveAction();

	UFUNCTION()
	void requestExecution_Implementation(AActor* Target);



	

};
