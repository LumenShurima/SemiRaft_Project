// Fill out your copyright notice in the Description page of Project Settings.


#include "Encounter/EncounterSubSystem.h"
#include "WaterBodyComponent.h"
#include "WaterSubsystem.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "InventorySystem/ItemBase.h"
#include "EngineUtils.h"
#include "ToolBuilderUtil.h"
#include "Weather/WeatherVolumetricCloud.h"
#include "Weather/WeatherWaterBodyOcean.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


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

AItemBase* UEncounterSubSystem::SpawnScrapEvent(APlayerController* PlayerController, TSubclassOf<AItemBase> ItemClass)
{
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Player Controller is Not Valid"),
			*GetClass()->GetName() , TEXT(__FUNCTION__));
		return nullptr;
	}
	
	APawn* Pawn = PlayerController->GetPawn();
	if (!Pawn)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Pawn is Not Valid"),
			*GetClass()->GetName() , TEXT(__FUNCTION__));
		return nullptr;
	}
	
	FVector PawnPos = Pawn->GetActorLocation();
	
	float Max = 1500.f;
	float U = FMath::FRandRange(0.0f, 1.0f);
	float V = FMath::FRandRange(0.0f, 1.0f);

	float R = Max;
	float Radius = R * FMath::Sqrt(U);
	float Theta = 2.0f * PI * V;

	float X = Radius * FMath::Cos(Theta);
	float Y = Radius * FMath::Sin(Theta);

	FVector Point = FVector(X, Y, 1000.f);
	
	FTransform SpawnTransform = FTransform::Identity;
	SpawnTransform.SetLocation(Point + PawnPos);
	
	return SpawnScrap(SpawnTransform, ItemClass);
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


