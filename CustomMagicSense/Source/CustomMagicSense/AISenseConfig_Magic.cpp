// Fill out your copyright notice in the Description page of Project Settings.


#include "AISenseConfig_Magic.h"
#include "AISense_Magic.h"

UAISenseConfig_Magic::UAISenseConfig_Magic(const FObjectInitializer& ObjectInitializer)
{
	DebugColor = FColor::Magenta;
}

TSubclassOf<UAISense> UAISenseConfig_Magic::GetSenseImplementation() const
{
	return UAISense_Magic::StaticClass();
}
