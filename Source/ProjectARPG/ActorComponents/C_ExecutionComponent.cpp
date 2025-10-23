// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ExecutionComponent.h"

// Sets default values for this component's properties
UC_ExecutionComponent::UC_ExecutionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UC_ExecutionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UC_ExecutionComponent::performExcution(APawn* pInstigator, APawn* pVictim)
{

}


// Called every frame
void UC_ExecutionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UC_ExecutionComponent::onBecomeExcutable(APawn* pVictim)
{

}

void UC_ExecutionComponent::tirggerExcution(APawn* pVictim)
{

}

