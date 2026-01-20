// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectARPG/Interface/C_ExecutionTarget.h"
#include "C_ExecutionComponent.generated.h"

enum class E_ExecutionType : uint8
{
	None,
	PostureBreak,
	Stealth
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_ExecutionComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UDataTable* m_pExecutionAnimsTable{};

	struct FS_ExecutionAnim* m_pExecutionAnims{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stun", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> m_pStunMontage;

	UPROPERTY()
	TScriptInterface<IC_ExecutionTarget> m_CurrentTarget;

	UPROPERTY()
	TWeakObjectPtr<AActor> m_CurrentTargetActor;

	UPROPERTY()
	TObjectPtr<class AC_PlayerCharacter> m_pOwnerPlayer;

	E_ExecutionType m_eCurrentExecutionType = E_ExecutionType::None;

	UPROPERTY()
	TArray<FS_ExecutionAnim> m_ExecutionMontages;

public:	
	// Sets default values for this component's properties
	UC_ExecutionComponent();

private:
	void updateExecutionTarget();
	void setCurrentExecutableTarget(AActor* NewActor, IC_ExecutionTarget* NewTarget, E_ExecutionType eType);
	bool isValidCurrentTarget() const;
	void findNewExecutionTarget();
	void clearCurrentTarget();
	bool isInStealthRange(AC_EnemyCharacter* pEnemy) const;
	bool isBehindTarget(AC_EnemyCharacter* pEnemy) const;
	bool canStealthExecute(AC_EnemyCharacter* pEnemy) const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void performExecution(APawn* pInstigator, APawn* pVictim, E_ExecutionType eType);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void onBecomeExecutable(APawn* pVictim);
	void triggerExecution(APawn* pVictim, E_ExecutionType eType);

	bool tryExecuteCurrentTarget();

	bool canStartExecution() const;
	E_ExecutionType getCurrentExecutionType() const;

	void playStunMontage();

	UFUNCTION()
	void onExecutionFinished(UAnimMontage* Montage, bool bInterrupted, class AC_EnemyCharacter* pVictim);

		
};
