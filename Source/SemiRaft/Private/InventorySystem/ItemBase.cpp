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
	
	
	
	if (bBuoyancyType)
	{
		if (GetRootComponent())
		{
			GetRootComponent()->SetVisibility(false);
			GetWorld()->GetTimerManager().SetTimer(
			SpawnTimerHandle,
			[this]()
			{
				if (!BuoyancyComponent) return;
				if (BuoyancyComponent->IsInWaterBody())
				{
					GetRootComponent()->SetVisibility(true);
				}
				else
				{
					Destroy();
				}
			},
			5.0f,
			false
			);
		}
	}
}

// Called every frame
void AItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}


