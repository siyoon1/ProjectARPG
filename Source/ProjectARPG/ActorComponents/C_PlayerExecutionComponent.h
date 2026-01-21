// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectARPG/Enums/C_ExecutionTypes.h"
#include "C_PlayerExecutionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_PlayerExecutionComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TObjectPtr<class AC_PlayerCharacter> m_OwnerPlayer;

	UPROPERTY()
	TWeakObjectPtr<AActor> m_CurrentTargetActor;

	UPROPERTY()
	TScriptInterface<class IC_ExecutionTarget> m_CurrentTarget;

	E_ExecutionType m_CurrentExecutionType = E_ExecutionType::None;

public:
	UC_PlayerExecutionComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	bool tryExecuteCurrentTarget();
	bool canStartExecution() const;
	E_ExecutionType getCurrentExecutionType() const;

private:
	void updateExecutionTarget();
	void findNewExecutionTarget();
	void clearCurrentTarget();
	bool selectExecution(E_ExecutionType Type, struct FS_ExecutionSelection& OutSelection);

	bool isValidCurrentTarget() const;
	void setCurrentExecutableTarget(AActor* NewActor, class IC_ExecutionTarget* NewTarget, E_ExecutionType Type);

	void performExecution(APawn* Instigator, APawn* Victim, E_ExecutionType Type);

		
};
