// Fill out your copyright notice in the Description page of Project Settings.


#include "Buoyancy/ExtendedBuoyancyComponent.h"


UExtendedBuoyancyComponent::UExtendedBuoyancyComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BuoyancyData.bCenterPontoonsOnCOM = false;
	BuoyancyData.bApplyDragForcesInWater = true;
	BuoyancyData.BuoyancyCoefficient = 1.0f;
	BuoyancyData.MaxBuoyantForce = TNumericLimits<float>::Max();
	
	AddCustomPontoon(100.0f, FVector(0.0f, 0.0f, 0.0f));
	AddCustomPontoon(100.0f, FVector(0.0f, 0.0f, 0.0f));
	AddCustomPontoon(100.0f, FVector(0.0f, 0.0f, 0.0f));
	AddCustomPontoon(100.0f, FVector(0.0f, 0.0f, 0.0f));
	
}

void UExtendedBuoyancyComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (SimulatingComponent)
	{
		if (bInitWithNeutralBuoyancy)
		{
			ComputePontoonsRadiusForNeutralBuoyancy();
		}
	}
}

void UExtendedBuoyancyComponent::ComputePontoonsRadiusForNeutralBuoyancy()
{
	double GravityZ = 0.0f;
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	GravityZ = World->GetGravityZ();
	
	if (!SimulatingComponent) return;
	
	double BodyMass = SimulatingComponent->GetMass();
	// Compute one radius for all pontoons because the body mass is distributed
	double PontoonRadius = FMath::Pow(6 * BodyMass * (-GravityZ), 1.f/3.f) / (2.f * FMath::Pow(PI, 1.f/3.f));
	for (FSphericalPontoon& Pontoon : BuoyancyData.Pontoons)
	{
		Pontoon.Radius = PontoonRadius;
	}
	
	UE_LOG(LogTemp, Error, TEXT("Pontoon Radius = %f"), BuoyancyData.Pontoons[0].Radius);
}
