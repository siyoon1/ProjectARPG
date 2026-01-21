// Fill out your copyright notice in the Description page of Project Settings.


#include "C_WallActionComponent.h"
#include "ProjectARPG/Character/C_PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "ProjectARPG/Animation/C_PlayerAnim.h"

// Sets default values for this component's properties
UC_WallActionComponent::UC_WallActionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UC_WallActionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UC_WallActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UC_WallActionComponent::init(AC_PlayerCharacter* InOwner)
{
	m_Owner = InOwner;
}

void UC_WallActionComponent::tickWallCheck()
{
	if (!m_Owner)
		return;

	checkWallTrace();
}

bool UC_WallActionComponent::tryWallGrab()
{
	if (!m_bCanWallGrab || m_bIsWallGrabbing)
		return false;

	m_bIsWallGrabbing = true;
	m_bCanWallGrab = false;
	
	m_Owner->setCombatState(E_CombatState::WallGrabbing);

	UCharacterMovementComponent* Move = m_Owner->GetCharacterMovement();


	Move->GravityScale = 0.f;
	Move->StopMovementImmediately();
	Move->AirControl = 1.f;
	Move->Velocity = FVector::ZeroVector;
	

	// 벽 바라보게 회전
	m_Owner->bUseControllerRotationYaw = false;
	m_Owner->SetActorLocation(m_Owner->GetActorLocation() - (-m_WallNormal * 5.f));

	return true;
}

void UC_WallActionComponent::releaseWall()
{
	if (!m_bIsWallGrabbing || !m_Owner)
		return;

	m_bIsWallGrabbing = false;
	m_bCanWallGrab = false;


	UCharacterMovementComponent* Move = m_Owner->GetCharacterMovement();

	if (Move)
	{
		// Falling 모드로 변경
		Move->SetMovementMode(MOVE_Falling);

		// 중력 복원
		Move->GravityScale = m_Owner->getDefaultGravity();

		// 공중 제어 초기화
		Move->AirControl = m_Owner->getDefaultAirControl();

		// Velocity 초기화
		Move->Velocity = FVector::ZeroVector;


	}

	m_Owner->bUseControllerRotationYaw = true;
	m_Owner->onActionFinished();

}

void UC_WallActionComponent::wallMove(const FVector2D& Input)
{
	if (!m_bIsWallGrabbing || !m_Owner)
		return;

	const float InputX = Input.X;
	if (FMath::IsNearlyZero(InputX))
		return;

	UCharacterMovementComponent* Move = m_Owner->GetCharacterMovement();
	UCapsuleComponent* Capsule = m_Owner->GetCapsuleComponent();

	// 중력 완전 차단
	Move->GravityScale = 0.f;

	// 회전 고정
	m_Owner->bUseControllerRotationYaw = false;
	Move->bOrientRotationToMovement = false;

	const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();

	// 벽 기준 좌우
	FVector WallRight =
		FVector::CrossProduct(m_WallNormal, FVector::UpVector).GetSafeNormal();

	FVector SweepDir = WallRight * InputX;

	// 머리 위치 기준
	FVector HeadPos =
		m_Owner->GetActorLocation() + FVector(0, 0, HalfHeight * 0.7f);

	FVector Start =
		HeadPos + SweepDir * 10.f + (-m_WallNormal) * 9.f;

	FVector End =
		HeadPos + SweepDir * 40.f + (-m_WallNormal) * 9.f;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(m_Owner);

	const float Radius = 30.f;

	bool bHit = GetWorld()->SweepSingleByChannel(
		Hit,
		Start,
		End,
		FQuat::Identity,
		ECC_GameTraceChannel4,
		FCollisionShape::MakeSphere(Radius),
		Params
	);

	// 벽이 계속 있을 때만 이동
	if (bHit)
	{
		FVector NewLoc =
			m_Owner->GetActorLocation() + SweepDir * 1.5f;

		m_Owner->SetActorLocation(NewLoc, true);
	}
}

bool UC_WallActionComponent::tryClimbUp()
{
	if (!m_bIsWallGrabbing)
		return false;

	FVector ClimbLoc = calcClimbLocation();
	UE_LOG(LogTemp, Warning, TEXT("ClimbLoc: %s"), *ClimbLoc.ToString());
	if (ClimbLoc == FVector::ZeroVector)
		return false;

	m_bIsWallGrabbing = false;

	m_Owner->setCombatState(E_CombatState::Climb);

	// Climb 목표 위치 세팅
	m_Owner->setClimbTarget(ClimbLoc);

	m_Owner->GetCharacterMovement()->SetMovementMode(MOVE_Flying);


	if (UC_PlayerAnim* Anim = Cast<UC_PlayerAnim>(m_Owner->GetMesh()->GetAnimInstance()))
	{
		Anim->playUpToClimb();
	}

	return true;
}

bool UC_WallActionComponent::canWallGrab() const
{
	return m_bCanWallGrab;
}


FVector UC_WallActionComponent::calcClimbLocation()
{
	if (!m_bIsWallGrabbing)
		return FVector::ZeroVector;

	float HalfHeight = m_Owner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float Radius = m_Owner->GetCapsuleComponent()->GetScaledCapsuleRadius();

	FVector ActorLoc = m_Owner->GetActorLocation();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(m_Owner);


	FVector SweepStart = ActorLoc + FVector(0, 0, HalfHeight * 0.4f) - m_WallNormal * 90.f;
	FVector SweepEnd = SweepStart + (-m_WallNormal * 70.f);

	FHitResult FrontHit;
	bool bFront = GetWorld()->SweepSingleByChannel(
		FrontHit,
		SweepStart,
		SweepEnd,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(Radius),
		Params
	);

	if (!bFront)
	{
		UE_LOG(LogTemp, Warning, TEXT("Front sweep fail"));
		return FVector::ZeroVector;
	}

	FVector TopStart = FrontHit.ImpactPoint + FVector(0, 0, 40.f);
	FVector TopEnd = TopStart - FVector(0, 0, 150.f);

	FHitResult DownHit;
	bool bDown = GetWorld()->LineTraceSingleByChannel
	(DownHit,
		TopStart,
		TopEnd,
		ECC_Visibility,
		Params
	);

	if (!bDown)
	{
		UE_LOG(LogTemp, Warning, TEXT("Down trace fail"));
		return FVector::ZeroVector;
	}

	float MaxClimbHeight = ActorLoc.Z + HalfHeight + 2.f;


	if (DownHit.ImpactPoint.Z > MaxClimbHeight)
	{
		UE_LOG(LogTemp, Warning, TEXT("Climb too high, fail"));
		return FVector::ZeroVector;
	}

	FVector FinalPos = DownHit.ImpactPoint;
	FinalPos.Z += HalfHeight;

	// Debug
	DrawDebugLine(GetWorld(), SweepStart, SweepEnd, FColor::Yellow, false, 2.f, 0, 2.f);
	DrawDebugLine(GetWorld(), TopStart, TopEnd, FColor::Cyan, false, 2.f, 0, 2.f);
	DrawDebugSphere(GetWorld(), FinalPos, 10.f, 12, FColor::Green, false, 2.f);

	return FinalPos;
}

bool UC_WallActionComponent::checkWallTrace()
{
	float HalfHeight = m_Owner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	// 벽 앞에서 상체 정도 높이에서 트레이스
	FVector Start = m_Owner->GetActorLocation() + FVector(0, 0, HalfHeight * 0.7f);
	FVector End = Start + m_Owner->GetActorForwardVector() * 80.f;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(m_Owner);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_GameTraceChannel4, Params
	);

	if (!m_bIsWallGrabbing)
		m_bCanWallGrab = bHit && Hit.Normal.Z < 0.5f;

	if (m_bCanWallGrab && !m_bIsWallGrabbing)
	{
		m_WallNormal = FVector(Hit.Normal.X, Hit.Normal.Y, 0.f).GetSafeNormal();
		m_WallHitLocation = Hit.Location;
	}

	return m_bCanWallGrab;

}

