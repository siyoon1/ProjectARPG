// Fill out your copyright notice in the Description page of Project Settings.


#include "C_BTDecorator_CheckMinRange.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ProjectARPG/AI/C_EnemyController.h"
#include "ProjectARPG/Character/C_EnemyCharacter.h"

UC_BTDecorator_CheckMinRange::UC_BTDecorator_CheckMinRange()
{
	NodeName = "Dist < MinRange";
}


