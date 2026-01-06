// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "C_BTTask_MoveToAttackRange.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTARPG_API UC_BTTask_MoveToAttackRange : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UC_BTTask_MoveToAttackRange();

	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

};
