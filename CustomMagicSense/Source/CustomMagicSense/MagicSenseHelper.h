// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense.h"
#include "AISenseConfig_Magic.h" // Make sure you include your custom sense config header here
#include "MagicSenseHelper.generated.h"

/**
 * 
 */
UCLASS()
class CUSTOMMAGICSENSE_API UMagicSenseHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** 
	 * Returns the UAISense instance matching the provided SenseClass from the PerceptionComponent's configured senses.
	 * Returns nullptr if none found.
	 */

	UFUNCTION(BlueprintCallable)
	static UAISense* GetSense(UAIPerceptionComponent* PerceptionComponent, TSubclassOf<UAISense> RequestedSense)
	{
		TArray<UAISenseConfig*>::TConstIterator It = PerceptionComponent->GetSensesConfigIterator();
		for (; It; ++It)
		{
			const UAISenseConfig* CurrentSenseConfig = *It;

			if (CurrentSenseConfig->GetSenseImplementation() == RequestedSense)
			{
				
			}
		}

		return nullptr;
	}

};
