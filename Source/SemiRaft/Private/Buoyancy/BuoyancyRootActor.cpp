// Fill out your copyright notice in the Description page of Project Settings.

#include "Buoyancy/BuoyancyRootActor.h"

#include "BuoyancyComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Buoyancy/ExtendedBuoyancyComponent.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
ABuoyancyRootActor::ABuoyancyRootActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;
	
	BuoyancyComponent = CreateDefaultSubobject<UExtendedBuoyancyComponent>(TEXT("BuoyancyComponent"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(
		TEXT("/Script/Engine.StaticMesh'/Game/FirstPerson/_GENERATED/Admin/Asset_Floor_Mesh.Asset_Floor_Mesh'")
	);
	
	if (MeshAsset.Succeeded())
	{
		auto Asset = MeshAsset.Object;
		FloorMesh = Asset;
	}
	
	RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PhysicsProxyMesh"));
	RootMesh->SetStaticMesh(FloorMesh);
	SetRootComponent(RootMesh);
	
	RootMesh->SetMobility(EComponentMobility::Movable);
	RootMesh->SetVisibility(false);
	RootMesh->SetHiddenInGame(true);

	RootMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RootMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	RootMesh->SetCollisionObjectType(ECC_PhysicsBody);
	RootMesh->SetCollisionResponseToAllChannels(ECR_Block);

	RootMesh->SetNotifyRigidBodyCollision(true);

	// BuoyancyComponent::SetupWaterBodyOverlaps()에서 Overlap 설정을 다시 만질 수 있음.
	// 여기서는 기본적으로 false로 시작해도 되지만, WaterBody overlap 문제가 있으면 true로 테스트.
	// RootMesh->SetGenerateOverlapEvents(true);

	RootMesh->SetSimulatePhysics(true);
	RootMesh->SetEnableGravity(true);

	// 빠르게 떨어지거나 작은 콜리전이면 지형을 관통할 수 있어서 우선 켜고 테스트
	RootMesh->BodyInstance.bUseCCD = true;
	
	
	
	
	// 생성자에서는 호출하지 말 것.
	// 컴포넌트 등록/BeginPlay 이전이라 부력 컴포넌트 내부 상태가 아직 완성되지 않았을 수 있음.
	// RebuildBuoyancyPontoons();

	if (BuoyancyComponent)
	{
		auto& Data = BuoyancyComponent->BuoyancyData;

		Data.bCenterPontoonsOnCOM = false;

		Data.BuoyancyDamp = 1000.0f;
		Data.BuoyancyDamp2 = 1.0f;
		Data.BuoyancyCoefficient = 2.0f;

		// 뗏목/건설 플랫폼이면 속도 기반 부력 램프는 과하면 튈 수 있음.
		// 필요하면 Blueprint/Details에서 따로 조절.
		// Data.BuoyancyRampMax = 1.0f;
	}
}

// Called when the game starts or when spawned
void ABuoyancyRootActor::BeginPlay()
{
	Super::BeginPlay();
	
	auto DefaultFloor = CreateFloorComponent(FIntPoint(0,0));
	
	FloorToGridMap.Add(DefaultFloor, FIntPoint(0,0));
	GridToFloorMap.Add(FIntPoint(0,0), DefaultFloor);
	
	RebuildBuoyancyPontoons();
}

// Called every frame
void ABuoyancyRootActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

UStaticMeshComponent* ABuoyancyRootActor::CreateFloorComponent(const FIntPoint& Grid)
{
	if (!FloorMesh || !RootComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateFloorComponent: Invalid FloorMesh or RootComponent."));
		return nullptr;
	}

	const FName ComponentName = MakeUniqueObjectName(
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
		UE_LOG(LogTemp, Error, TEXT("CreateFloorComponent: Failed to create NewFloor."));
		return nullptr;
	}

	NewFloor->SetStaticMesh(FloorMesh);
	NewFloor->SetMobility(EComponentMobility::Movable);

	// 자식 Floor는 독립 물리 바디로 굴리지 않는다.
	// RootMesh 하나만 물리 시뮬레이션을 담당하게 두는 쪽이 안정적.
	NewFloor->SetSimulatePhysics(false);
	NewFloor->SetEnableGravity(false);

	// 건설 위치 판정 / 라인 트레이스용이면 QueryOnly 권장.
	// 물리 충돌을 RootMesh 하나에만 맡길지, 자식 Floor도 QueryAndPhysics로 둘지는 설계 선택.
	// NewFloor->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	NewFloor->SetCollisionResponseToAllChannels(ECR_Block);
	NewFloor->SetGenerateOverlapEvents(false);

	const float LocalX = Grid.X * GridSize;
	const float LocalY = Grid.Y * GridSize;

	// 런타임 생성 컴포넌트는 먼저 Attach하고 RelativeLocation을 세팅한 뒤 Register하는 쪽이 안전함.
	FAttachmentTransformRules AttachRules(
		EAttachmentRule::KeepRelative,
		EAttachmentRule::KeepRelative,
		EAttachmentRule::KeepRelative,
		true
	);

	NewFloor->AttachToComponent(RootComponent, AttachRules);
	NewFloor->SetRelativeLocation(FVector(LocalX, LocalY, 0.0f));

	NewFloor->RegisterComponent();
	

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
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("ABuoyancyRootActor::AddFloor: Target floor not found: %s"),
			*TargetFloor->GetName()
		);
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
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("ABuoyancyRootActor::AddFloor: Grid already occupied. Grid(%d, %d)"),
			NewGrid.X,
			NewGrid.Y
		);
		return nullptr;
	}

	UStaticMeshComponent* NewFloor = CreateFloorComponent(NewGrid);

	if (!NewFloor)
	{
		return nullptr;
	}

	FloorToGridMap.Add(NewFloor, NewGrid);
	GridToFloorMap.Add(NewGrid, NewFloor);
	
	RebuildBuoyancyPontoons();

	return NewFloor;
}

void ABuoyancyRootActor::RemoveFloor(UStaticMeshComponent* TargetFloor)
{
	if (!TargetFloor)
	{
		return;
	}

	// 기준 바닥은 제거하지 않음.
	if (TargetFloor == RootMesh)
	{
		return;
	}

	const FIntPoint* Grid = FloorToGridMap.Find(TargetFloor);

	if (!Grid)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("ABuoyancyRootActor::RemoveFloor: Target floor not found: %s"),
			*TargetFloor->GetName()
		);
		return;
	}

	const FIntPoint GridCopy = *Grid;

	FloorToGridMap.Remove(TargetFloor);
	GridToFloorMap.Remove(GridCopy);

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
	
	auto& Pontoons = BuoyancyComponent->BuoyancyData.Pontoons;

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

	if (GridToFloorMap.Num() <= 0)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("ABuoyancyRootActor::RebuildBuoyancyPontoons: No floor grid exists.")
		);
		return;
	}

	int32 MinGridX = TNumericLimits<int32>::Max();
	int32 MaxGridX = TNumericLimits<int32>::Lowest();
	int32 MinGridY = TNumericLimits<int32>::Max();
	int32 MaxGridY = TNumericLimits<int32>::Lowest();

	for (const TPair<FIntPoint, UStaticMeshComponent*>& Pair : GridToFloorMap)
	{
		const FIntPoint& Grid = Pair.Key;
		UStaticMeshComponent* FloorComp = Pair.Value;

		if (!FloorComp)
		{
			continue;
		}

		MinGridX = FMath::Min(MinGridX, Grid.X);
		MaxGridX = FMath::Max(MaxGridX, Grid.X);
		MinGridY = FMath::Min(MinGridY, Grid.Y);
		MaxGridY = FMath::Max(MaxGridY, Grid.Y);
	}

	if (MinGridX == TNumericLimits<int32>::Max() ||
		MinGridY == TNumericLimits<int32>::Max() ||
		MaxGridX == TNumericLimits<int32>::Lowest() ||
		MaxGridY == TNumericLimits<int32>::Lowest())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("ABuoyancyRootActor::RebuildBuoyancyPontoons: No valid floor component exists.")
		);
		return;
	}

	const float HalfGrid = GridSize * 0.5f;

	const float MinX = static_cast<float>(MinGridX) * GridSize - HalfGrid;
	const float MaxX = static_cast<float>(MaxGridX) * GridSize + HalfGrid;
	const float MinY = static_cast<float>(MinGridY) * GridSize - HalfGrid;
	const float MaxY = static_cast<float>(MaxGridY) * GridSize + HalfGrid;

	const float PontoonZ = Pontoons[0].RelativeLocation.Z;

	FVector WorldCentorPos = RootMesh->GetCenterOfMass();
	
	FVector RelativleCentorPos = RootMesh->GetComponentTransform().InverseTransformPosition(WorldCentorPos);
	
	const float LocalMinX = RelativleCentorPos.X - 250;
	const float LocalMaxX = RelativleCentorPos.X + 250;
	const float LocalMinY = RelativleCentorPos.Y - 250;
	const float LocalMaxY = RelativleCentorPos.Y + 250;

	Pontoons[0].RelativeLocation = FVector(LocalMinX, LocalMinY, PontoonZ);
	Pontoons[1].RelativeLocation = FVector(LocalMaxX, LocalMinY, PontoonZ);
	Pontoons[2].RelativeLocation = FVector(LocalMinX, LocalMaxY, PontoonZ);
	Pontoons[3].RelativeLocation = FVector(LocalMaxX, LocalMaxY, PontoonZ);
	
	BuoyancyComponent->ComputePontoonsRadiusForNeutralBuoyancy();
	

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("RebuildBuoyancyPontoons: Grid Min(%d, %d), Max(%d, %d), Bounds Min(%f, %f), Max(%f, %f), Z(%f)"),
		MinGridX,
		MinGridY,
		MaxGridX,
		MaxGridY,
		MinX,
		MinY,
		MaxX,
		MaxY,
		PontoonZ
	);
}
