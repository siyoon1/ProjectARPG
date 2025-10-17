// Fill out your copyright notice in the Description page of Project Settings.


#include "C_ARPGGameMode.h"

AC_ARPGGameMode::AC_ARPGGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> Character(TEXT("/Game/Blueprints/Character/Player/bp-PlayerCharacter.bp-PlayerCharacter_C"));
	if (Character.Succeeded())
	{
		DefaultPawnClass = Character.Class;
	}

	static ConstructorHelpers::FClassFinder<APlayerController> Controller(TEXT("/Game/Blueprints/Controller/bp-PlayerController.bp-PlayerController_C"));
	if (Controller.Succeeded())
	{
		PlayerControllerClass = Controller.Class;
	}
}
