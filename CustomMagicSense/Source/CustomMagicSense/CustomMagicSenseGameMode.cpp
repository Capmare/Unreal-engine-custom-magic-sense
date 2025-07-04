// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomMagicSenseGameMode.h"
#include "CustomMagicSenseCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACustomMagicSenseGameMode::ACustomMagicSenseGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
