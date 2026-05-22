// Fill out your copyright notice in the Description page of Project Settings.


#include "Encounter/EncounterSubSystem.h"
#include "WaterBodyComponent.h"
#include "WaterSubsystem.h"
#include "ShallowWaterSubsystem.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "InventorySystem/ItemBase.h"
#include "EngineUtils.h"
#include "Components/VolumetricCloudComponent.h"


void UEncounterSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	WorldInitHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, 
		&UEncounterSubSystem::PostWorldInit);
	
}

void UEncounterSubSystem::Deinitialize()
{
	FWorldDelegates::OnPostWorldInitialization.Remove(WorldInitHandle);
	
	
	Super::Deinitialize();
}

void UEncounterSubSystem::Tick(float DeltaTime)
{
	if (!IsTickable())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: %s"),
			*GetClass()->GetName() , TEXT(__FUNCTION__)
			,TEXT("Tickable is False"));
		return;
	}
}

void UEncounterSubSystem::PostWorldInit(UWorld* World, const UWorld::InitializationValues IVS)
{
	if (!World || !World->IsGameWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: %s"),
			*GetClass()->GetName() , TEXT(__FUNCTION__)
			,TEXT("World OR GameWorld is Not Valid"));
		return;
	}
	
	World->OnWorldBeginPlay.AddUObject(
		this,
		&UEncounterSubSystem::PostBeginPlay);
}

void UEncounterSubSystem::PostBeginPlay()
{
	
}

UVolumetricCloudComponent* UEncounterSubSystem::GetVolumetricCloudComponent()
{
	UWorld* World = GetWorld();
	if (!World || !World->IsGameWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: %s"),
			*GetClass()->GetName() , TEXT(__FUNCTION__)
			,TEXT("World OR GameWorld is Not Valid"));
		return nullptr;
	}

	AVolumetricCloud* FoundCloud = nullptr; 

	for (TActorIterator<AVolumetricCloud> It(World); It; ++It)
	{
		FoundCloud = *It;
		break;
	}

	if (!FoundCloud)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: %s"),
			*GetClass()->GetName() , TEXT(__FUNCTION__)
			,TEXT("Coulld not find the volumetric cloud."));
		return nullptr;
	}

	VolumetricCloudComponent = FoundCloud->GetComponentByClass<UVolumetricCloudComponent>();
	if (!VolumetricCloudComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: %s"),
			*GetClass()->GetName() , TEXT(__FUNCTION__)
			,TEXT("Volumetric Cloud Component is Not Valid."));
		return nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("%s::%s: %s"),
			*GetClass()->GetName() , TEXT(__FUNCTION__)
			,TEXT("Volumetric Cloud Component is Valid."));
	
	return VolumetricCloudComponent;
}

UEncounterSubSystem* UEncounterSubSystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("UEncounterSubSystem: Invalid WorldContextObject"));
		return nullptr;
	}
	
	const UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("UEncounterSubSystem: Invalid World"));
		return nullptr;
	}
	
	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("UEncounterSubSystem: Invalid GameInstance."));
		return nullptr;
	}
	
	UE_LOG(LogTemp, Log, TEXT("UEncounterSubSystem: Get Game Instance"));
	return GameInstance->GetSubsystem<UEncounterSubSystem>();
}

AItemBase* UEncounterSubSystem::SpawnScrap(FTransform SpawnTransform, TSubclassOf<AItemBase> ItemClass)
{
	UWorld* World = GetWorld();
	
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: %s"),
			*GetClass()->GetName() , TEXT(__FUNCTION__)
			,TEXT("World is Not Valid"));
		return nullptr;
	}
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	Params.TransformScaleMethod = ESpawnActorScaleMethod::OverrideRootScale;
	auto SpawnItem = World->SpawnActor<AItemBase>(ItemClass, SpawnTransform, Params);
	
	if (!SpawnItem)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: %s"),
			*GetClass()->GetName() , TEXT(__FUNCTION__)
			,TEXT("Spawn Item is NULL"));
		return nullptr;
	}
	
	UE_LOG(LogTemp, Error, 
		TEXT("[Spawn Item]\n"
			"Name : %s\n"
			"Location : X : %f, Y : %f, Z : %f"),
			*SpawnItem->GetName(),
			SpawnTransform.GetLocation().X,
			SpawnTransform.GetLocation().Y,
			SpawnTransform.GetLocation().Z
			);
	
	return SpawnItem;
}


