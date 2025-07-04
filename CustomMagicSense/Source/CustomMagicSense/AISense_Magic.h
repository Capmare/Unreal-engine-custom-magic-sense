// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Perception/AISense.h"
#include "Containers/Map.h" 
#include "AISenseConfig_Magic.h"
#include "AISense_Magic.generated.h"

/**
 *
 */

class UAISenseConfig_Magic;

// https://www.orfeasel.com/creating-custom-ai-senses/
USTRUCT(BlueprintType)
struct FDigestedMagicProperties {
    GENERATED_USTRUCT_BODY()


    float DetectionRadius{};
    float Sensitivity{1.f};
    TSet<EMagicType> AllowedTypes{};
    float MinPerceivedStrength{ 0.1 };
    bool bCanIgnoreLowStrengthEvent{ false };
	bool bShouldUseTeams{false};
	FGenericTeamId TeamIdentifier{FGenericTeamId::NoTeam};

    FDigestedMagicProperties();
    FDigestedMagicProperties(const UAISenseConfig_Magic& SenseConfig);
};



USTRUCT(BlueprintType)
struct CUSTOMMAGICSENSE_API FAIMagicEvent
{
    GENERATED_USTRUCT_BODY()

    typedef class UAISense_Magic FSenseClass;

	UPROPERTY(BlueprintReadWrite)
    TWeakObjectPtr<AActor> Instigator{nullptr};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense")
    FVector MagicLocation{FVector::ZeroVector};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense")
    EMagicType MagicType{EMagicType::Elemental};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense")
    float MagicPower = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense")
    float Timestamp = 0.f;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense")
	float ThreatLevel = 0.5f; // 0 = curious, 1 = dangerous

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense")
	float EventLifetime = 3.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense")
    FGenericTeamId TeamIdentifier = FGenericTeamId::NoTeam;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense")
    float MaxRange{1000};


    // need default constructor too to work, cannot delete defautl constructor because UHT calls FAIMagicEvent(void)
    FAIMagicEvent()
        : Instigator(nullptr), MagicLocation(FVector::ZeroVector), MagicType(EMagicType::Elemental), MagicPower(1.f), Timestamp(0.f)
    {}

    FAIMagicEvent(AActor* InInstigator, const FVector& InLocation)
        : Instigator(InInstigator), MagicLocation(InLocation), MagicType(EMagicType::Elemental), MagicPower(1.f), Timestamp(0.f)
    {
        if (InInstigator)
        {
            if (IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(Instigator))
            {
                TeamIdentifier = TeamAgent->GetGenericTeamId();
            }
        }

    }
};

// this class is similar to the hearing class 
UCLASS()
class CUSTOMMAGICSENSE_API UAISense_Magic : public UAISense
{
    GENERATED_BODY()
public:
    UAISense_Magic(const FObjectInitializer& ObjectInitializer);

    // implement bindings for the listener
    void OnNewListener(const FPerceptionListener& NewListener);
    void OnListenerUpdate(const FPerceptionListener& UpdatedListener);
    void OnListenerRemoved(const FPerceptionListener& UpdatedListener);


    UFUNCTION(BlueprintCallable, Category = "AI|Perception", meta = (WorldContext="WorldContextObject"))
    static void ReportMagicEvent(UObject* WorldContextObject, AActor* Instigator, FVector Location, EMagicType MagicType, FGenericTeamId Team, float Power, float ThreatLevel, float Lifetime = 3.f);


    // Register the magic event
    void RegisterEvent(const FAIMagicEvent& Event);

    virtual float Update() override;

private:
    // List of all magic events executed this frame
    UPROPERTY()
    TArray<FAIMagicEvent> MagicEvents;

    // Store digested properties for each listener
    TMap<FPerceptionListenerID, FDigestedMagicProperties> DigestedProperties;
    
    const float EventLifetime = 5.0f; 
};