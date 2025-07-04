// Fill out your copyright notice in the Description page of Project Settings.

#include "AISense_Magic.h"
#include "AISenseConfig_Magic.h"
#include "Perception/AIPerceptionComponent.h" 
#include "UObject/NoExportTypes.h"

void UAISense_Magic::RegisterEvent(const FAIMagicEvent& Event)
{

    FAIMagicEvent TimedEvent = Event;
    TimedEvent.Timestamp = GetWorld()->GetTimeSeconds();
    MagicEvents.Add(TimedEvent);
    // Request update from the Update()
    RequestImmediateUpdate();
}

float UAISense_Magic::Update()
{

    const float CurrentTime = GetWorld()->GetTimeSeconds();
    AIPerception::FListenerMap& ListenersMap = *GetListeners();
    
    for (auto& ListenerPair : ListenersMap)
    {

        FPerceptionListener& Listener = ListenerPair.Value;
    
        if (!Listener.HasSense(GetSenseID()))
            continue;
    
        const FVector ListenerLocation = Listener.CachedLocation;
        const FDigestedMagicProperties* ListenerProps = DigestedProperties.Find(Listener.GetListenerID());
        if (!ListenerProps)
            continue; // Skip if we can't find the listener's config

        for (int32 i = MagicEvents.Num() - 1; i >= 0; --i)
        {
            FAIMagicEvent& Event = MagicEvents[i];
            AActor* InstigatorActor = Event.Instigator.Get();

            if (!IsValid(InstigatorActor))
            {
                continue;
            }
    
            // Remove expired events
            if (CurrentTime - Event.Timestamp > Event.EventLifetime)
            {
                MagicEvents.RemoveAt(i);
                continue;
            }

            if (ListenerProps->bShouldUseTeams)
            {
                FGenericTeamId ListenerTeamId{ FGenericTeamId::NoTeam };
				if (const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(Listener.GetBodyActor()))
				{
					ListenerTeamId = TeamAgent->GetGenericTeamId();
				}

				// ignore friendlies
				if (ListenerTeamId == Event.TeamIdentifier)
				{
					continue;
				}
            }
            

            // Filter by distance
            float DistanceSquared = FVector::DistSquared(Event.MagicLocation, ListenerLocation);
            DrawDebugSphere(GetWorld(), Event.MagicLocation, ListenerProps->DetectionRadius, 6, FColor::Purple, false, 1.5f);
            if (DistanceSquared > FMath::Square(ListenerProps->DetectionRadius))
                continue;
    
            // Filter by allowed magic types
            if (!ListenerProps->AllowedTypes.Contains(Event.MagicType))
                continue;
    
            FHitResult HitResult;
            FCollisionQueryParams Params(SCENE_QUERY_STAT(AISenseMagic), true);
            Params.bReturnPhysicalMaterial = true;
            Params.AddIgnoredActor(InstigatorActor);
    
            bool bBlocked = GetWorld()->LineTraceSingleByChannel(
                HitResult,
                Event.MagicLocation,
                ListenerLocation,
                ECC_Visibility,
                Params
            );
    
            float Strength = 1.0f;
    
            // Check if magic is blocked or dampened by the environment
            if (bBlocked)
            {
                DrawDebugSphere(GetWorld(), HitResult.Location, 20.0f, 6, FColor::Purple, false, 1.5f);

                if (HitResult.PhysMaterial.IsValid())
                {
                    switch (HitResult.PhysMaterial->SurfaceType)
                    {
                    case SurfaceType1:
                        continue;
                        break;
                    case SurfaceType2:
                        Strength *= 0.5f;
                        break;
                    case SurfaceType3:
                        Strength *= 1.5f;
                        break;

                    default:
                        break;
                    }
                }
            }
    
            Strength *= ListenerProps->Sensitivity;
    
            // Build a tag string to pass threat level info into stimulus
            FString TagString = FString::Printf(TEXT("Threat=%.2f"), Event.ThreatLevel);
    
            FString MagicTypeStr = UEnum::GetValueAsString(Event.MagicType);

			UE_LOG(LogTemp, Log, TEXT("[MagicSense] Listener: %s | Instigator: %s | MagicType: %s | Strength: %.2f | ThreatLevel: %.2f | Blocked: %s"),
				*Listener.GetBodyActorName().ToString(),
				*GetNameSafe(InstigatorActor),
				*MagicTypeStr,
				Strength,
				Event.ThreatLevel,
				bBlocked ? TEXT("Yes") : TEXT("No"));
    

            float DistanceSq = FVector::DistSquared(ListenerLocation, Event.MagicLocation);

            // Range based on power
            float EffectiveRange = Event.MaxRange * Event.MagicPower;
            float EffectiveRangeSq = FMath::Square(EffectiveRange);
            
            if (ListenerProps->bCanIgnoreLowStrengthEvent)
            {
                if (DistanceSq <= EffectiveRangeSq)
				{
					float Distance = FMath::Sqrt(DistanceSq);
					float AttenuationFactor = 1.0f - (Distance / EffectiveRange);
					AttenuationFactor = FMath::Clamp(AttenuationFactor, 0.0f, 1.0f);
					float FinalStrength = Strength * AttenuationFactor;

					// accept only stronger signals
					if (FinalStrength >= ListenerProps->MinPerceivedStrength)
					{
						FAIStimulus Stimulus(*this, FinalStrength, Event.MagicLocation, ListenerLocation, FAIStimulus::SensingSucceeded, FName(*TagString));
						Listener.RegisterStimulus(InstigatorActor, Stimulus);

						// draw debug stuff
						DrawDebugLine(GetWorld(), Event.MagicLocation, ListenerLocation, bBlocked ? FColor::Red : FColor::Green, false, 1.0f);
						DrawDebugSphere(GetWorld(), Event.MagicLocation, 20.0f, 6, FColor::Purple, false, 1.5f);

						// Draw debug string above the magic location showing strength and threat
						DrawDebugString(GetWorld(), Event.MagicLocation + FVector(0, 0, 30),
							FString::Printf(TEXT("Strength: %.2f\nThreat: %.2f"), FinalStrength, Event.ThreatLevel),
							nullptr, FColor::White, 1.5f);
					}
				}
            }
            else
            {
                FAIStimulus Stimulus(*this, Strength, Event.MagicLocation, ListenerLocation, FAIStimulus::SensingSucceeded, FName(*TagString));
				Listener.RegisterStimulus(InstigatorActor, Stimulus);

				// draw debug stuff
				DrawDebugLine(GetWorld(), Event.MagicLocation, ListenerLocation, bBlocked ? FColor::Red : FColor::Green, false, 1.0f);
				DrawDebugSphere(GetWorld(), Event.MagicLocation, 20.0f, 6, FColor::Purple, false, 1.5f);

				// Draw debug string above the magic location showing strength and threat
				DrawDebugString(GetWorld(), Event.MagicLocation + FVector(0, 0, 30),
					FString::Printf(TEXT("Strength: %.2f\nThreat: %.2f"), Strength, Event.ThreatLevel),
							nullptr, FColor::White, 1.5f);
            }            
        }
    }
    
    // UAI_Sense uses FLT_MAX, this will make sure the sense doesn't update every frame but only when registering a new sense
    return SuspendNextUpdate;

}

UAISense_Magic::UAISense_Magic(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    // set bindings
    OnNewListenerDelegate.BindUObject(this, &UAISense_Magic::OnNewListener);
    OnListenerUpdateDelegate.BindUObject(this, &UAISense_Magic::OnListenerUpdate);
    OnListenerRemovedDelegate.BindUObject(this, &UAISense_Magic::OnListenerRemoved);
}


// Code for adding, updating and removing listeners
// https://www.orfeasel.com/creating-custom-ai-senses/
void UAISense_Magic::OnNewListener(const FPerceptionListener& NewListener)
{
    UE_LOG(LogTemp, Display, TEXT("New Magic listener"))

    UAIPerceptionComponent* ListenerPtr = NewListener.Listener.Get();
    check(ListenerPtr);

    const UAISenseConfig_Magic* SenseConfig = Cast<const UAISenseConfig_Magic>(ListenerPtr->GetSenseConfig(GetSenseID()));
    check(SenseConfig);

    const FDigestedMagicProperties PropertyDigest(*SenseConfig);
    DigestedProperties.Add(NewListener.GetListenerID(), PropertyDigest);
}

void UAISense_Magic::OnListenerUpdate(const FPerceptionListener& UpdatedListener)
{
    const FPerceptionListenerID ListenerID = UpdatedListener.GetListenerID();

    // make sure is a valid listener
    if (UpdatedListener.HasSense(GetSenseID()))
    {
        UAIPerceptionComponent* ListenerPtr = UpdatedListener.Listener.Get();
        if (ListenerPtr)
        {
            const UAISenseConfig_Magic* SenseConfig = Cast<const UAISenseConfig_Magic>(ListenerPtr->GetSenseConfig(GetSenseID()));
            check(SenseConfig);
            FDigestedMagicProperties& PropertiesDigest = DigestedProperties.FindOrAdd(ListenerID, FDigestedMagicProperties());
            PropertiesDigest = FDigestedMagicProperties(*SenseConfig);
        }
    }
    else
    {
        DigestedProperties.Remove(ListenerID);
    }
}

void UAISense_Magic::OnListenerRemoved(const FPerceptionListener& UpdatedListener)
{
    // safely remove the listener
    DigestedProperties.FindAndRemoveChecked(UpdatedListener.GetListenerID());
}



void UAISense_Magic::ReportMagicEvent(UObject* WorldContextObject, AActor* Instigator, FVector Location, EMagicType MagicType,FGenericTeamId TeamIdentifier,  float Power, float ThreatLevel, float Lifetime /*= 3.f*/)
{
    UWorld* World = Cast<UWorld>(WorldContextObject->GetWorld());

    if (!Instigator || !World)
    {
        return;
    }

    FAIMagicEvent NewEvent;
    NewEvent.Instigator = Instigator;
    NewEvent.MagicLocation = Location;
    NewEvent.MagicType = MagicType;
    NewEvent.MagicPower = Power;
    NewEvent.ThreatLevel = ThreatLevel;
    NewEvent.EventLifetime = Lifetime;
    NewEvent.Timestamp = World->GetTimeSeconds();
    NewEvent.TeamIdentifier = TeamIdentifier;

    UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(WorldContextObject);
	if (PerceptionSystem)
	{
		PerceptionSystem->OnEvent(NewEvent);
	}
    
}

FDigestedMagicProperties::FDigestedMagicProperties() : DetectionRadius(0.0f)
{

}

FDigestedMagicProperties::FDigestedMagicProperties(const UAISenseConfig_Magic& SenseConfig)
{
    DetectionRadius = SenseConfig.MagicDetectionRadius;
    Sensitivity = SenseConfig.Sensitivity;
    AllowedTypes = SenseConfig.AllowedTypes;
    MinPerceivedStrength = SenseConfig.MinPerceivedStrength;
    bCanIgnoreLowStrengthEvent = SenseConfig.bCanIgnoreLowStrengthEvent;
    bShouldUseTeams = SenseConfig.bShouldUseTeams;
    TeamIdentifier = SenseConfig.TeamIdentifier;
}
