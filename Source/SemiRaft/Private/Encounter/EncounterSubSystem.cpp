// Fill out your copyright notice in the Description page of Project Settings.


#include "Encounter/EncounterSubSystem.h"
#include "WaterBodyComponent.h"
#include "WaterSubsystem.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "InventorySystem/ItemBase.h"
#include "EngineUtils.h"
#include "BuildSystem/RaftActor.h"
#include "Weather/WeatherVolumetricCloud.h"
#include "Weather/WeatherWaterBodyOcean.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Enemy/Shark.h"
#include "Enemy/JawsSpline.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UEncounterSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	WorldInitHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(
		this,
		&UEncounterSubSystem::PostWorldInit
	);

	JawsSound = LoadObject<USoundBase>(
		nullptr,
		TEXT("/Game/Sound/jaws_Shark_Cue.jaws_Shark_Cue")
	);

	SharkClass = LoadClass<AShark>(
		nullptr,
		TEXT("/Game/FirstPerson/Blueprints/BP_FuckingShark.BP_FuckingShark_C")
	);

	JawsSplineClass = LoadClass<AJawsSpline>(
		nullptr,
		TEXT("/Game/FirstPerson/Blueprints/BP_Spline.BP_Spline_C")
	);

	if (!JawsSound)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load JawsSound"));
	}

	if (!SharkClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load SharkClass"));
	}

	if (!JawsSplineClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load JawsSplineClass"));
	}
	
	ItemPool.SetNum(ItemPoolSize);
	
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

void UEncounterSubSystem::SpawnJawsEncounter(AActor* TargetActor)
{
	if (!IsValid(TargetActor))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: TargetActor is invalid."),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
		return;
	}

	UWorld* World = TargetActor->GetWorld();
	if (!IsValid(World))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: World is invalid."),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
		return;
	}

	if (!JawsSplineClass)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: JawsSplineClass is null."),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
		return;
	}

	if (!SharkClass)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: SharkClass is null."),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
		return;
	}

	FActorSpawnParameters Params;
	Params.Owner = TargetActor;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	Params.TransformScaleMethod =
		ESpawnActorScaleMethod::OverrideRootScale;

	const FVector SpawnLocation = TargetActor->GetActorLocation();

	AJawsSpline* JawsSpline = World->SpawnActor<AJawsSpline>(
		JawsSplineClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		Params
	);

	if (!IsValid(JawsSpline))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Failed to spawn JawsSpline."),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
		return;
	}

	if (!IsValid(JawsSpline->TargetSphereComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: JawsSpline TargetSphereComponent is invalid."),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
		JawsSpline->Destroy();
		return;
	}

	JawsSpline->AttachToActor(
		TargetActor,
		FAttachmentTransformRules::KeepRelativeTransform
	);

	AShark* Shark = World->SpawnActor<AShark>(
		SharkClass,
		JawsSpline->TargetSphereComponent->GetComponentLocation(),
		FRotator::ZeroRotator,
		Params
	);

	if (!IsValid(Shark))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Failed to spawn Shark."),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
		JawsSpline->Destroy();
		return;
	}

	Shark->SetTarget(JawsSpline->TargetSphereComponent);

	if (IsValid(JawsSound))
	{
		UGameplayStatics::PlaySound2D(World, JawsSound);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%s: JawsSound is null."),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
	}

	const float RandDuration = FMath::RandRange(10.0f, 23.0f);

	TWeakObjectPtr<AShark> WeakShark = Shark;
	TWeakObjectPtr<UWorld> WeakWorld = World;
	TWeakObjectPtr<AJawsSpline> WeakSpline = JawsSpline;

	FTimerHandle TimerHandle;

	World->GetTimerManager().SetTimer(
	TimerHandle,
	[WeakShark, WeakWorld, WeakSpline]()
	{
			AShark* Shark = WeakShark.Get();
			if (!IsValid(Shark))
			{
				return;
			}

			UWorld* TimerWorld = WeakWorld.Get();
			if (!IsValid(TimerWorld))
			{
				return;
			}
		
		// 1순위: Player Character가 Swimming 상태면 플레이어 추적
		ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(TimerWorld, 0);

		if (IsValid(PlayerCharacter))
		{
			UCharacterMovementComponent* CharacterMovement = PlayerCharacter->GetCharacterMovement();

			if (IsValid(CharacterMovement) && CharacterMovement->MovementMode == MOVE_Swimming)
			{
				USceneComponent* CharacterRootComponent = PlayerCharacter->GetRootComponent();

				if (IsValid(CharacterRootComponent))
				{
					Shark->SetTarget(CharacterRootComponent);

					if (WeakSpline.IsValid())
					{
						WeakSpline->Destroy();
					}

					return;
				}
			}
		}

		// 2순위: 플레이어가 Swimming 상태가 아니면 RaftActor 추적
		ARaftActor* RaftActor = Cast<ARaftActor>(
			UGameplayStatics::GetActorOfClass(
				TimerWorld,
				ARaftActor::StaticClass()
			)
		);

		if (IsValid(RaftActor))
		{
			USceneComponent* RaftRootComponent = RaftActor->GetRootComponent();

			if (IsValid(RaftRootComponent))
			{
				Shark->SetTarget(RaftRootComponent);

				if (WeakSpline.IsValid())
				{
					WeakSpline->Destroy();
				}

				return;
			}
		}

			
		},
		RandDuration,
		false
	);
}

