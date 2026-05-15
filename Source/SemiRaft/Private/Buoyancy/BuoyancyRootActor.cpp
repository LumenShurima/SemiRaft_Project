// Fill out your copyright notice in the Description page of Project Settings.


#include "Buoyancy/BuoyancyRootActor.h"
#include "BuoyancyComponent.h"
#include "BodyInstanceCore.h"

// Sets default values
ABuoyancyRootActor::ABuoyancyRootActor()
{
	PrimaryActorTick.bCanEverTick = true;

	RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));
	SetRootComponent(RootMesh);

	BuoyancyComponent = CreateDefaultSubobject<UBuoyancyComponent>(TEXT("BuoyancyComponent"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(
		TEXT("/Script/Engine.StaticMesh'/Game/FirstPerson/_GENERATED/Admin/Asset_Floor_Mesh.Asset_Floor_Mesh'")
	);

	if (MeshAsset.Succeeded())
	{
		FloorMesh = MeshAsset.Object;
		RootMesh->SetStaticMesh(FloorMesh);
	}

	RootMesh->SetMobility(EComponentMobility::Movable);

	RootMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RootMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	RootMesh->SetCollisionObjectType(ECC_PhysicsBody);
	RootMesh->SetCollisionResponseToAllChannels(ECR_Block);

	RootMesh->SetNotifyRigidBodyCollision(true);
	RootMesh->SetGenerateOverlapEvents(false);

	RootMesh->SetSimulatePhysics(true);
	RootMesh->SetEnableGravity(true);

	// 빠르게 떨어지거나 작은 콜리전이면 지형을 관통할 수 있어서 우선 켜고 테스트
	RootMesh->BodyInstance.bUseCCD = true;

	// 생성자에서는 기본 맵만 세팅
	FloorToGridMap.Add(RootMesh, FIntPoint(0, 0));
	GridToFloorMap.Add(FIntPoint(0, 0), RootMesh);

	// 여기서 호출하지 말 것
	// RebuildBuoyancyPontoons();
	
	auto& Data = BuoyancyComponent->BuoyancyData;

	Data.bCenterPontoonsOnCOM = false;
	
	Data.BuoyancyDamp = 2000.0f;
	Data.BuoyancyDamp2 = 1.0f;
	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;
	
	
}

// Called when the game starts or when spawned
void ABuoyancyRootActor::BeginPlay()
{
	Super::BeginPlay();
	RebuildBuoyancyPontoons();
}

// Called every frame
void ABuoyancyRootActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	ClampPhysicsVelocity();

}

UStaticMeshComponent* ABuoyancyRootActor::CreateFloorComponent(const FIntPoint& Grid)
{
	if (!FloorMesh || !RootComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateFloorComponent: Invalid FloorMesh or RootComponent."));
		return nullptr;
	}

	FName ComponentName = MakeUniqueObjectName(
		this,
		UStaticMeshComponent::StaticClass(),
		TEXT("Floor")
	);

	UStaticMeshComponent* NewFloor = NewObject<UStaticMeshComponent>(
		this,
		UStaticMeshComponent::StaticClass(),
		ComponentName
	);

	if (!NewFloor)
	{
		return nullptr;
	}

	NewFloor->SetStaticMesh(FloorMesh);

	// 자식 Floor는 독립 물리 바디로 굴리지 않는 쪽이 안정적
	NewFloor->SetSimulatePhysics(false);
	NewFloor->SetEnableGravity(false);

	// 건설 위치 판정 / 라인 트레이스용이면 QueryOnly 권장
	NewFloor->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	const float LocalX = Grid.X * GridSize;
	const float LocalY = Grid.Y * GridSize;

	NewFloor->SetRelativeLocation(FVector(LocalX, LocalY, 0.0f));

	NewFloor->RegisterComponent();

	FAttachmentTransformRules AttachRules(
		EAttachmentRule::KeepRelative,
		EAttachmentRule::KeepRelative,
		EAttachmentRule::KeepRelative,
		true // bWeldSimulatedBodies
	);

	NewFloor->AttachToComponent(RootComponent, AttachRules);

	return NewFloor;
}

UStaticMeshComponent* ABuoyancyRootActor::AddFloor(UStaticMeshComponent* TargetFloor, EFloorDirection Direction)
{
	if (!TargetFloor)
	{
		return nullptr;
	}

	const FIntPoint* TargetGrid = FloorToGridMap.Find(TargetFloor);

	if (!TargetGrid)
	{
		UE_LOG(LogTemp, Warning, TEXT("ABuoyancyRootActor::AddFloor = Target floor not found: %s"), *TargetFloor->GetName());
		return nullptr;
	}

	FIntPoint NewGrid = *TargetGrid;
	
	switch (Direction)
	{
	case EFloorDirection::North:
		NewGrid.Y += 1;
		break;

	case EFloorDirection::South:
		NewGrid.Y -= 1;
		break;

	case EFloorDirection::East:
		NewGrid.X += 1;
		break;

	case EFloorDirection::West:
		NewGrid.X -= 1;
		break;

	default:
		return nullptr;
	}

	if (GridToFloorMap.Contains(NewGrid))
	{
		return nullptr;
	}

	UStaticMeshComponent* NewFloor = CreateFloorComponent(NewGrid);

	if (!NewFloor)
	{
		return nullptr;
	}

	FloorToGridMap.Add(NewFloor, NewGrid);
	GridToFloorMap.Add(NewGrid, NewFloor);
	
	BuoyancyComponent->BuoyancyData.BuoyancyCoefficient += BuoyancyCoefficientBias;
	
	
	RebuildBuoyancyPontoons();
	
	return NewFloor;
}

void ABuoyancyRootActor::RemoveFloor(UStaticMeshComponent* TargetFloor)
{
	if (!TargetFloor)
	{
		return;
	}
	
	if (TargetFloor == RootMesh)
	{
		return;
	}

	FIntPoint RemovedGrid;

	if (!FloorToGridMap.RemoveAndCopyValue(TargetFloor, RemovedGrid))
	{
		return;
	}

	GridToFloorMap.Remove(RemovedGrid);
	BuoyancyComponent->BuoyancyData.BuoyancyCoefficient -= BuoyancyCoefficientBias;
	TargetFloor->DestroyComponent();

	RebuildBuoyancyPontoons();
}

void ABuoyancyRootActor::RebuildBuoyancyPontoons()
{
	if (!BuoyancyComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("RebuildBuoyancyPontoons: Invalid BuoyancyComponent."));
		return;
	}

	BuoyancyComponent->BuoyancyData.Pontoons.Reset();

	TSet<FIntPoint> PontoonSet;

	for (const TPair<TObjectPtr<UStaticMeshComponent>, FIntPoint>& Pair : FloorToGridMap)
	{
		UStaticMeshComponent* FloorComponent = Pair.Key.Get();

		if (!FloorComponent)
		{
			continue;
		}

		const FIntPoint& Grid = Pair.Value;

		AddPontoonUnique(PontoonSet, FIntPoint(Grid.X * 2 - 1, Grid.Y * 2 + 1)); // NorthWest
		AddPontoonUnique(PontoonSet, FIntPoint(Grid.X * 2 + 1, Grid.Y * 2 + 1)); // NorthEast
		AddPontoonUnique(PontoonSet, FIntPoint(Grid.X * 2 - 1, Grid.Y * 2 - 1)); // SouthWest
		AddPontoonUnique(PontoonSet, FIntPoint(Grid.X * 2 + 1, Grid.Y * 2 - 1)); // SouthEast
	}
	
	const int PontoonCount = BuoyancyComponent->BuoyancyData.Pontoons.Num();
	
	
	if (PontoonCount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RebuildBuoyancyPontoons: No pontoons generated."));
		return;
	}
	
	const float PontoonCountFloat =  static_cast<float>(PontoonCount);
	
	auto& Data = BuoyancyComponent->BuoyancyData;
	
	float FixedMassKg = 100.f;
	float GravityCm = 980.0f;
	float BuoyancyRatio = 1.3f;
	float BaseBuoyancyDamp = 1500.f;
	float BaseBuoyancyDamp2 = 1.1f;
	
	float TargetTotalBuoyancy = (FixedMassKg * GravityCm * BuoyancyRatio) / 100000.f;

	Data.bCenterPontoonsOnCOM = false;
	
	float FinalBuoyancy = TargetTotalBuoyancy /PontoonCountFloat;
	Data.BuoyancyCoefficient = FinalBuoyancy;
	Data.BuoyancyDamp  = BaseBuoyancyDamp  / (PontoonCountFloat);
	Data.BuoyancyDamp2 = BaseBuoyancyDamp2 / (PontoonCountFloat);
	
	
	UE_LOG(LogTemp, Log, TEXT("RebuildBuoyancyPontoons: Pontoon count = %d"), PontoonCount);
	UE_LOG(LogTemp, Log, TEXT("RebuildBuoyancyPontoons: Buoyancy Coefficient Divide Value = %f"), FinalBuoyancy);
}
	

void ABuoyancyRootActor::AddPontoonUnique(TSet<FIntPoint>& PontoonSet, const FIntPoint& PontoonGrid)
{
	if (PontoonSet.Contains(PontoonGrid))
	{
		return;
	}

	PontoonSet.Add(PontoonGrid);

	const float HalfGridSize = GridSize * 0.5f;
	
	

	FSphericalPontoon NewPontoon;
	NewPontoon.RelativeLocation = FVector(
		PontoonGrid.X * HalfGridSize,
		PontoonGrid.Y * HalfGridSize,
		-10.f
	);
	UE_LOG(LogTemp, Warning, TEXT("Pontoon Local = %s Radius = %f"),
		*NewPontoon.RelativeLocation.ToString(),
		NewPontoon.Radius);

	BuoyancyComponent->BuoyancyData.Pontoons.Add(NewPontoon);
}

void ABuoyancyRootActor::ClampPhysicsVelocity()
{
		
	if (!RootMesh || !RootMesh->IsSimulatingPhysics())
	{
		return;
	}

	FVector Velocity = RootMesh->GetPhysicsLinearVelocity();

	const float DeltaZ = Velocity.Z - PrevLinearSpeed.Z;

	if (DeltaZ < MaxDownwardSpeed || DeltaZ > MaxUpwardSpeed)
	{
		Velocity.Z = 0;
		UE_LOG(LogTemp, Warning,
			TEXT("ABuoyancyRootActor::ClampPhysicsVelocity: Abnormal velocity detected. Linear velocity has been reset to zero."));
	}

	RootMesh->SetPhysicsLinearVelocity(Velocity, false);

	PrevLinearSpeed = Velocity;
}

