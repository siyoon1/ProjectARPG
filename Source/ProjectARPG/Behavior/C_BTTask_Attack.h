// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "C_BTTask_Attack.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTARPG_API UC_BTTask_Attack : public UBTTaskNode
{
	GENERATED_BODY()

private:
	UPROPERTY()
	UBehaviorTreeComponent* CachedOwnerComp = nullptr;

public:
	UC_BTTask_Attack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory
	) override;

	void onAttackEnded();
	
};
