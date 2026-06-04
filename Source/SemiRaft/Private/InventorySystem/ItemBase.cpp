// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/ItemBase.h"
#include "InventorySystem/InventorySubsystem.h"

#include "EnvironmentQuery/EnvQueryDebugHelpers.h"
#include "Buoyancy/ExtendedBuoyancyComponent.h"
// Sets default values
AItemBase::AItemBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	BuoyancyComponent = CreateDefaultSubobject<UExtendedBuoyancyComponent>(TEXT("BuoyancyComponent"));
	
	
}

// Called when the game starts or when spawned
void AItemBase::BeginPlay()
{
	Super::BeginPlay();

	if (!bBuoyancyType)
	{
		return;
	}

	UWorld* World = GetWorld();
	USceneComponent* Root = GetRootComponent();

	if (!World || !Root)
	{
		return;
	}

	// 루트뿐 아니라 자식 컴포넌트까지 같이 숨김
	Root->SetVisibility(false, true);

	World->GetTimerManager().SetTimer(
		SpawnTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			UWorld* InnerWorld = GetWorld();
			USceneComponent* InnerRoot = GetRootComponent();

			if (!InnerWorld || !InnerRoot)
			{
				Destroy();
				return;
			}

			if (!IsValid(BuoyancyComponent))
			{
				Destroy();
				return;
			}

			if (!BuoyancyComponent->IsInWaterBody())
			{
				Destroy();
				return;
			}

			InnerRoot->SetVisibility(true, true);

			const float RandDestroyTime = FMath::RandRange(20.f, 40.f);

			InnerWorld->GetTimerManager().SetTimer(
				SinkTimerHandle,
				FTimerDelegate::CreateWeakLambda(this, [this]()
				{
					UWorld* InnerWorld = GetWorld();

					if (!InnerWorld)
					{
						return;
					}

					if (!IsValid(BuoyancyComponent))
					{
						Destroy();
						return;
					}

					BuoyancyComponent->BuoyancyData.BuoyancyCoefficient = 0.6f;

					InnerWorld->GetTimerManager().SetTimer(
						FinalDestroyTimerHandle,
						FTimerDelegate::CreateWeakLambda(this, [this]()
						{
							Destroy();
						}),
						3.f,
						false
					);
				}),
				RandDestroyTime,
				false
			);
		}),
		5.0f,
		false
	);
}

void AItemBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();

		TimerManager.ClearTimer(SpawnTimerHandle);
		TimerManager.ClearTimer(SinkTimerHandle);
		TimerManager.ClearTimer(FinalDestroyTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}


