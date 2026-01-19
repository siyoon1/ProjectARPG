// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_WallActionComponent.generated.h"

class AC_PlayerCharacter;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTARPG_API UC_WallActionComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	AC_PlayerCharacter* m_Owner = nullptr;

	bool m_bIsWallGrabbing = false;
	bool m_bCanWallGrab = false;

	float m_fDefaultGravity = 2.f;
	float m_fDefaultAirControl = 0.35f;

	FVector m_WallNormal;
	FVector m_WallHitLocation;

public:	
	// Sets default values for this component's properties
	UC_WallActionComponent();

	void init(AC_PlayerCharacter* InOwner);

	void tickWallCheck(); // 벽 체크
	bool tryWallGrab(); // 벽짚기 시도
	void releaseWall(); // 벽짚기 해제
	void wallMove(const FVector2D& Input); // 벽 좌우 이동
	bool tryClimbUp(); // 벽 등반

	inline bool isWallGrabbing() const { return m_bIsWallGrabbing; }
	bool canWallGrab() const;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	bool checkWallTrace();
	FVector calcClimbLocation();
};
