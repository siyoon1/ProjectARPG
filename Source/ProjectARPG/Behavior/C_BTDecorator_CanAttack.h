// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "C_BTDecorator_CanAttack.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTARPG_API UC_BTDecorator_CanAttack : public UBTDecorator
{
	GENERATED_BODY()


public:
	UC_BTDecorator_CanAttack();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
};
