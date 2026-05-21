// Fill out your copyright notice in the Description page of Project Settings.


#include "Buoyancy/RaftPlatformBuoyancyComponent.h"
#include "Buoyancy/BuoyancyRootActor.h"

URaftPlatformBuoyancyComponent::URaftPlatformBuoyancyComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BuoyancyData.bCenterPontoonsOnCOM = false;
	BuoyancyData.bApplyDragForcesInWater = true;
	BuoyancyData.BuoyancyCoefficient = 2.0f;
	BuoyancyData.MaxBuoyantForce = TNumericLimits<float>::Max();
	
	AddCustomPontoon(100.0f, FVector(0.0f, 0.0f, 0.0f));
	AddCustomPontoon(100.0f, FVector(0.0f, 0.0f, 0.0f));
	AddCustomPontoon(100.0f, FVector(0.0f, 0.0f, 0.0f));
	AddCustomPontoon(100.0f, FVector(0.0f, 0.0f, 0.0f));
	
}

void URaftPlatformBuoyancyComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (SimulatingComponent)
	{
		if (bInitWithNeutralBuoyancy)
		{
			ComputePontoonsForRaft();
		}
	}
}

void URaftPlatformBuoyancyComponent::ComputePontoonsForRaft()
{
	double GravityZ = 0.0f;
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	if (!SimulatingComponent) return;
	
	
	auto& Pontoons = BuoyancyData.Pontoons;
	
	if (Pontoons.Num() < 4)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("ABuoyancyRootActor::RebuildBuoyancyPontoons: Need at least 4 pontoons. Current: %d"),
			Pontoons.Num()
		);
		return;
	}
	TArray<USceneComponent*> Floors;
	SimulatingComponent->GetChildrenComponents(false, Floors);
	
	FVector BoundingMin = FVector(1,1,0) * TNumericLimits<double>::Max();
	FVector BoundingMax = FVector(1,1,0) * TNumericLimits<double>::Lowest();
	
	bool bFoundFloor = false;

	for (USceneComponent* Floor : Floors)
	{
		// 추후에 floor class Component로 캐스팅 변경해야함
		UStaticMeshComponent* FloorMesh = Cast<UStaticMeshComponent>(Floor);
		if (!FloorMesh)
		{
			continue;
		}

		const FVector FloorPos = FloorMesh->GetRelativeLocation();

		BoundingMin.X = FMath::Min(BoundingMin.X, FloorPos.X);
		BoundingMax.X = FMath::Max(BoundingMax.X, FloorPos.X);
		BoundingMin.Y = FMath::Min(BoundingMin.Y, FloorPos.Y);
		BoundingMax.Y = FMath::Max(BoundingMax.Y, FloorPos.Y);

		bFoundFloor = true;
	}

	if (!bFoundFloor)
	{
		BoundingMin = FVector(0,0,0);
		BoundingMax = FVector(0,0,0);
	}
	
	GravityZ = World->GetGravityZ();
	;
	FVector WorldCenterOfMass = SimulatingComponent->GetCenterOfMass();
	
	FVector RelativeCentorOfMass = SimulatingComponent->GetComponentTransform().InverseTransformPosition(WorldCenterOfMass);
	
	
	const float DistanceLeft  = FMath::Abs(RelativeCentorOfMass.X - BoundingMin.X);
	const float DistanceRight = FMath::Abs(BoundingMax.X - RelativeCentorOfMass.X);

	const float DistanceBack  = FMath::Abs(RelativeCentorOfMass.Y - BoundingMin.Y);
	const float DistanceFront = FMath::Abs(BoundingMax.Y - RelativeCentorOfMass.Y);

	const float ExtentX = FMath::Max(DistanceLeft, DistanceRight);
	const float ExtentY = FMath::Max(DistanceBack, DistanceFront);
	
	const float HalfGridSzie = GridSize * 0.5;
	/* Min */
	const float FinalMinX = RelativeCentorOfMass.X - ExtentX - HalfGridSzie;
	const float FinalMinY = RelativeCentorOfMass.Y - ExtentY - HalfGridSzie;
	/* Max */
	const float FinalMaxX = RelativeCentorOfMass.X + ExtentX + HalfGridSzie;
	const float FinalMaxY = RelativeCentorOfMass.Y + ExtentY + HalfGridSzie;
	
	
	Pontoons[0].RelativeLocation = FVector(FinalMinX, FinalMinY, PontoonZ);
	Pontoons[1].RelativeLocation = FVector(FinalMaxX, FinalMinY, PontoonZ);
	Pontoons[2].RelativeLocation = FVector(FinalMinX, FinalMaxY, PontoonZ);
	Pontoons[3].RelativeLocation = FVector(FinalMaxX, FinalMaxY, PontoonZ);
	
	
	double BodyMass = SimulatingComponent->GetMass();
	// Compute one radius for all pontoons because the body mass is distributed
	double PontoonRadius = FMath::Pow(6 * BodyMass * (-GravityZ), 1.f/3.f) / (2.f * FMath::Pow(PI, 1.f/3.f));
	for (FSphericalPontoon& Pontoon : Pontoons)
	{
		Pontoon.Radius = PontoonRadius;
	}
}