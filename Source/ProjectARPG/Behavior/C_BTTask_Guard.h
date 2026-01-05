// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "C_BTTask_Guard.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTARPG_API UC_BTTask_Guard : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UC_BTTask_Guard();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory
	) override;
	
};
