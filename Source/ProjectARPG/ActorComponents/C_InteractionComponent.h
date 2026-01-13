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
	class AC_PlayerCharacter* m_CachedPlayer;

	UPROPERTY()
	class AC_NPCCharacter* m_Owner;


public:	
	// Sets default values for this component's properties
	UC_InteractionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void showIneractUI();

	void hideInteractUI();

	UFUNCTION(BlueprintCallable)
	void onPlayerEnter(AC_PlayerCharacter* Player);

	UFUNCTION(BlueprintCallable)
	void onPlayerExit();

	bool canInteract() const;


		
};
