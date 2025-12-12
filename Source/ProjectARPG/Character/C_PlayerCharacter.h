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
	class AC_EnemyCharacter* m_pCurrentExecutionTarget = nullptr;

	//플레이어 연속 공격 관련 변수
	int32 m_nCurrentComboIndex = 0;
	int32 m_nMaxComboIndex = 5;
	bool m_bNextComboQueued = false;
	float m_fLastAttackInputTime = 0.f;
	float m_fInputBuffer = 0.25f;

	//플레이어 기본 이동속도
	float m_fDefaultSpeed = 800.f;

	//플레이어 락온 관련 변수
	AC_CombatCharacter* m_pCurrentLockOnTarget{};
	bool m_bIsLockOn = false;

	//플레이어 클라이밍
	FVector2D m_vCurrentMoveInput{};
	FVector m_vWallNormal{};
	FVector m_vWallHitLocation{};
	FVector m_vClimbLocation{};
	bool m_bJumpPressed = false; // 점프가 눌리고 있는지
	bool m_bCanWallGrab = false; // 벽을 짚을수 있는지
	bool m_bIsWallGrabbing = false; // 벽을 짚고 있는지
	bool m_bCanClimbUp = false; // 벽을 올라갈수 있는지

	int32 m_nJumpCount = 0;
	int32 m_MaxJumpCount = 2;


	//플레이어 웅크리기 관련 변수
	bool m_bIsCrouch = false;


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
	//콤보 공격 실행
	void playCombo(int32 nComboIndex);

	//공격이 가능한지 체크
	bool canAttack() const;

	//전방에 적 찾기
	AActor* getCurrentEnemy();

	//락온 대상 찾기
	AC_CombatCharacter* findLockOnTarget();

	//락온 지정
	void setLockOn(float fDelta);
	
	//벽 짚기 가능한지 확인하기
	void checkWallTrace();

	bool canGrabWallAtLoc(const FVector& checkLoc);

	//벽 짚기
	void setWallGrab(bool bEnable);

	//위쪽 지면 감지하기
	FVector checkClimbableSurface();

	//벽 올라가기
	void startClimbUp();

	//착지 상태
	void Landed(const FHitResult& Hit) override;

	//벽 좌우 이동
	void wallGrabMove(const FVector2D& MoveInput);

	//인살 시도 함수
	bool tryExcuteEnemy() const;

	//인살 가능한 적 감지하는 함수
	void checkExecutionCandidate();

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

public:
	void setCombatState(E_CombatState eNewState) override;
	void onComboTransition();
	void resetCombo();



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

	UFUNCTION(BlueprintCallable)
	bool isPulling() const;

	//벽짚은 위치
	FVector getClimbLoc() const;

	bool isPlayerControlled() const;

	UCameraComponent* getFollowCamera() const;

	void initJump();

};
