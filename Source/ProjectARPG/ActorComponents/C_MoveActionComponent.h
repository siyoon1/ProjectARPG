// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_MoveActionComponent.generated.h"

class AC_PlayerCharacter;
class UC_SprintComponent;
class UC_DodgeComponent;
class UC_WallActionComponent;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_MoveActionComponent : public UActorComponent
{
	GENERATED_BODY()


private:
	UPROPERTY()
	AC_PlayerCharacter* m_Owner = nullptr;

	UPROPERTY()
	UC_SprintComponent* m_SprintComp = nullptr;

	UPROPERTY()
	UC_DodgeComponent* m_DodgeComp = nullptr;

	UPROPERTY()
	UC_WallActionComponent* m_WallActionComp = nullptr;

	// 입력 판정
	UPROPERTY(EditDefaultsOnly)
	float m_DodgeTapThreshold = 0.3f;

public:	
	// Sets default values for this component's properties
	UC_MoveActionComponent();

	void init(AC_PlayerCharacter* InOwner);

	void onSprintInput(const struct FInputActionInstance& Inst);

	void onSprintReleased();

	void interruptMovementAction();

	bool tryWallGrab();

	void wallMove(const FVector2D& Input);

	bool isWallGrabbing() const;

	bool canWallGrab() const;

	bool tryWallJumpOrClimb();

	void handleClimbclamp();

private:
	bool tryDodge(const FInputActionInstance& Inst);
	bool trySprint();


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
