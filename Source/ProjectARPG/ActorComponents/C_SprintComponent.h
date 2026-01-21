// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_SprintComponent.generated.h"

UENUM()
enum class E_SprintEndReason
{
	InputReleased,
	Attack,
	Dodge,
	Guard,
	Parry,
	Hit,
	WallGrab,
	Forced
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_SprintComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY()
	class AC_PlayerCharacter* m_Owner = nullptr;

	UPROPERTY()
	class AC_PlayerCameraManager* m_CameraMgr;

	bool m_bIsSprinting = false;
	float m_DefaultSpeed = 620.f;
	float m_SprintSpeed = 1000.f;

private:
	void restoreMovement();

public:	
	// Sets default values for this component's properties
	UC_SprintComponent();

	void init(AC_PlayerCharacter* InOwner);

	bool canStartSprint() const;

	bool startSprint();
	void stopSprint(E_SprintEndReason Reason);
	void forceStop();

	inline bool isSprinting() const { return m_bIsSprinting; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
