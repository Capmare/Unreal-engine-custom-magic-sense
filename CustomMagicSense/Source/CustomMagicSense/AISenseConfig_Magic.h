// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Perception/AISenseConfig.h"

#include "AISenseConfig_Magic.generated.h"

// Pimpl the config so we dont have to reload the state tree on each modification we do for the sense code

UENUM(BlueprintType)
enum class EMagicType : uint8 { Elemental , Fire, Necromancy, Illusion };

UCLASS(meta = (DisplayName = "AI Magic config"), BlueprintType)
class CUSTOMMAGICSENSE_API UAISenseConfig_Magic : public UAISenseConfig
{
	GENERATED_BODY()
public:
	UAISenseConfig_Magic(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sense")
	float MagicDetectionRadius{200.f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sense")
	float Sensitivity{1.f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sense")
    TSet<EMagicType> AllowedTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense")
	float MinPerceivedStrength = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense")
	bool bCanIgnoreLowStrengthEvent{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense")
	bool bShouldUseTeams{false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense")
	FGenericTeamId TeamIdentifier{FGenericTeamId::NoTeam};

	virtual TSubclassOf<UAISense> GetSenseImplementation() const override;
};
