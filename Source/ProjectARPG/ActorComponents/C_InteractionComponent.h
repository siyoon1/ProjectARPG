// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_InteractionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_InteractionComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY()
	AActor* m_CurrentTarget;

	UPROPERTY()
	TArray<AActor*> m_InteractableList;



private:
	void updateCurrentTarget();

public:	
	// Sets default values for this component's properties
	UC_InteractionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool tryInteract();

	UFUNCTION(BlueprintCallable)
	void showUI(AActor* Target);

	UFUNCTION(BlueprintCallable)
	void hideUI(AActor* Target);

	
	UFUNCTION(BlueprintCallable)
	void registerInteractable(AActor* Actor);
	UFUNCTION(BlueprintCallable)
	void unregisterInteractable(AActor* Actor);
};
