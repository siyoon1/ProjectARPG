// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_DodgeComponent.generated.h"

class AC_PlayerCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_DodgeComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY()
	AC_PlayerCharacter* m_Owner = nullptr;

public:	
	// Sets default values for this component's properties
	UC_DodgeComponent();

	void init(AC_PlayerCharacter* InOwner);

	bool tryDodge();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
