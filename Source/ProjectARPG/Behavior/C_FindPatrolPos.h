// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "C_FindPatrolPos.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTARPG_API UC_FindPatrolPos : public UBTService
{
	GENERATED_BODY()

public:
	UC_FindPatrolPos();

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
};
