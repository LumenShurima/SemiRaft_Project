// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildSystem/RaftActor.h"

#include "BuildSystem/BuildComponent.h"
#include "BuildSystem/FloorComp.h"
#include "BuildSystem/RoofComp.h"
#include "BuildSystem/WallComp.h"
#include "Buoyancy/RaftPlatformBuoyancyComponent.h"


// Sets default values
ARaftActor::ARaftActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GridSize = 100.f;
	RootMesh = CreateDefaultSubobject<UStaticMeshComponent>("RootMesh");
	SetRootComponent(RootMesh);
	RootMesh->SetSimulatePhysics(true);
	RootMesh->SetCollisionProfileName(TEXT("Building"));
	RootMesh->SetVisibility(false);
	RootMesh->SetHiddenInGame(true);
	DynamicAnchor = FIntVector(0, 0, 0);
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (MeshAsset.Succeeded())
	{
		DefaultFloorMesh = MeshAsset.Object;
	}
	
	BuildComponent = CreateDefaultSubobject<UBuildComponent>(TEXT("BuildComponent"));
	BuoyancyComponent = CreateDefaultSubobject<URaftPlatformBuoyancyComponent>(TEXT("BuoyancyComponent"));
	TArray<FIntVector> InitialCoordinates = {
		FIntVector(0, 0, 0), // 좌측 하단
		FIntVector(1, 0, 0), // 우측 하단
		FIntVector(0, 1, 0), // 좌측 상단
		FIntVector(1, 1, 0)  // 우측 상단
	};

	for (const FIntVector& Coord : InitialCoordinates)
	{
		FString UniqueCompName = FString::Printf(TEXT("InitialFloor_%d_%d_%d"), Coord.X, Coord.Y, Coord.Z);

		UFloorComp* NewFloorComp = CreateDefaultSubobject<UFloorComp>(FName(*UniqueCompName));

		if (NewFloorComp)
		{
			NewFloorComp->SetupAttachment(RootComponent);

			if (DefaultFloorMesh)
			{
				NewFloorComp->SetStaticMesh(DefaultFloorMesh);
				NewFloorComp->SetMaterial(0, FloorMaterial);
			}
			NewFloorComp->SetWorldScale3D(FVector(1.f, 1.f, 0.5f));
			NewFloorComp->SetCollisionProfileName(TEXT("Building"));
			NewFloorComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

			FVector RelativePosition(Coord.X * GridSize, Coord.Y * GridSize, Coord.Z * GridSize);
			NewFloorComp->SetRelativeLocation(RelativePosition);
			NewFloorComp->GridKey = Coord;

			GridMap.Add(Coord, NewFloorComp);
		}
	}
	/*if (BuoyancyComponent)
	{
		auto& Data = BuoyancyComponent->BuoyancyData;

		Data.bCenterPontoonsOnCOM = false;

		Data.BuoyancyDamp = 1000.0f;
		Data.BuoyancyDamp2 = 1.0f;
		Data.BuoyancyCoefficient = 2.0f;
		

		// 뗏목/건설 플랫폼이면 속도 기반 부력 램프는 과하면 튈 수 있음.
		// 필요하면 Blueprint/Details에서 따로 조절.
		// Data.BuoyancyRampMax = 1.0f;
	}*/
	
}

// Called when the game starts or when spawned
void ARaftActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ARaftActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!RootMesh)
	{
		return;
	}

	const FVector MoveDirection = GetActorRightVector().GetSafeNormal();
	const float TargetSpeed = InstalledEngine ? EngineTargetSpeed : BaseTargetSpeed;
	const FVector CurrentVelocity = RootMesh->GetPhysicsLinearVelocity();
	const float CurrentForwardSpeed = FVector::DotProduct(CurrentVelocity, MoveDirection);
	const float SpeedError = TargetSpeed - CurrentForwardSpeed;
	const float Acceleration = FMath::Clamp(
		SpeedError * SpeedControlGain,
		-MaxAcceleration,
		MaxAcceleration
	);

	RootMesh->AddForce(MoveDirection * Acceleration, NAME_None, true);
}

bool ARaftActor::InstallEngine()
{
	if (InstalledEngine || !EngineClass || GridMap.IsEmpty())
	{
		return false;
	}

	int32 MinY = MAX_int32;
	int32 MinX = MAX_int32;
	int32 MaxX = MIN_int32;

	for (const auto& Pair : GridMap)
	{
		const FIntVector& Grid = Pair.Key;
		MinY = FMath::Min(MinY, Grid.Y);
		MinX = FMath::Min(MinX, Grid.X);
		MaxX = FMath::Max(MaxX, Grid.X);
	}

	const float CenterX = (MinX + MaxX) * 0.5f;
	FVector SpawnLocation = GetActorTransform().TransformPosition(
		FVector(CenterX * GridSize, MinY * GridSize, 0.f)
	);
	SpawnLocation += -GetActorRightVector() * EngineBackOffset;
	SpawnLocation += GetActorUpVector() * EngineUpOffset;

	const FRotator SpawnRotation = GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	InstalledEngine = GetWorld()->SpawnActor<AActor>(
		EngineClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (!InstalledEngine)
	{
		return false;
	}

	InstalledEngine->AttachToComponent(RootMesh, FAttachmentTransformRules::KeepWorldTransform);
	return true;
}

void ARaftActor::SpawnFloorAtGrid(FIntVector TargetGrid)
{
	FString CompName = FString::Printf(TEXT("FloorBlock_%d_%d_%d"), TargetGrid.X, TargetGrid.Y, TargetGrid.Z);
	UFloorComp* NewFloor = NewObject<UFloorComp>(this, FName(*CompName));
	NewFloor->SetWorldScale3D(FVector(1.f, 1.f, 0.5f));
    
	if (NewFloor)
	{
		if (RootMesh)
		{
			NewFloor->SetupAttachment(RootMesh);
		}
		else
		{
			// RootMesh 비어있으면 루트찾아서 붙임
			NewFloor->SetupAttachment(GetRootComponent());
		}

		// 부착 후 등록
		NewFloor->RegisterComponent();

		// 스케일 조절
		NewFloor->SetWorldScale3D(FVector(1.f, 1.f, 0.5f));
        
		if (DefaultFloorMesh) 
		{
			NewFloor->SetStaticMesh(DefaultFloorMesh);
			NewFloor->SetMaterial(0, FloorMaterial);
		}
		NewFloor->SetCollisionProfileName(TEXT("Building"));
		NewFloor->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		
		float Size = GridSize; 
		FVector FloorLocalPos(
			TargetGrid.X * Size,
			TargetGrid.Y * Size,
			0.f // 바닥 레이어는 평면 고정
		);
        
		// 로컬 좌표 설정
		NewFloor->SetRelativeLocation(FloorLocalPos);
		NewFloor->GridKey = TargetGrid;

		// 주소록에 등록
		GridMap.Add(TargetGrid, NewFloor);
	}
	BuoyancyComponent->ComputePontoonsForRaft();
	DynamicAnchor = FindDynamicAnchorPoint();
	UpdateEdgeGridSet();
}


void ARaftActor::SpawnWallAtGrid(FIntVector TargetGrid)
{
	FString CompName = FString::Printf(TEXT("WallBlock_%d_%d_%d"), TargetGrid.X, TargetGrid.Y, TargetGrid.Z);
	UWallComp* NewWall = NewObject<UWallComp>(this, FName(*CompName));

	if (NewWall)
	{
		// 바닥의 가상그리드 맵 키로 map에서 메시컴포넌트 찾아서 저장해주기
		const FIntVector FloorKey(TargetGrid.X, TargetGrid.Y, 0);
		UFloorComp** FloorComp = GridMap.Find(FloorKey);
		if (!FloorComp || !*FloorComp)
		{
			NewWall->DestroyComponent();
			return;
		}

		NewWall->Floor = *FloorComp;
		NewWall->FloorKey = FloorKey;
		NewWall->WallKey = TargetGrid;
		if (RootMesh)
		{
			NewWall->SetupAttachment(RootMesh);
		}
		else
		{
			NewWall->SetupAttachment(GetRootComponent());
		}
		NewWall->RegisterComponent();

		NewWall->SetWorldScale3D(FVector(1.f, 1.f, 1.f));

		if (WallMesh)
		{
			NewWall->SetStaticMesh(WallMesh);
			NewWall->SetMaterial(0, WallMaterial);
		}
		NewWall->SetSimulatePhysics(false);
		NewWall->SetMassOverrideInKg(NAME_None, 0.f, true);
		NewWall->SetCollisionProfileName(TEXT("Building"));
		NewWall->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		NewWall->SetCollisionResponseToAllChannels(ECR_Block);
		NewWall->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		NewWall->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

		// BuildComponent에서 계산한 프리뷰 위치/회전 그대로 사용
		NewWall->SetRelativeLocation(BuildComponent->PreviewMeshComp->GetRelativeLocation());
		NewWall->SetRelativeRotation(BuildComponent->PreviewMeshComp->GetRelativeRotation());

		WallMap.Add(TargetGrid, NewWall);
		NewWall->Floor->AddWalls(NewWall);
	}
}

void ARaftActor::SpawnRoofAtGrid(FIntVector TargetGrid, UWallComp* TargetWall)
{
	if (!TargetWall || TargetWall->Roof || RoofMap.Contains(TargetWall->WallKey)) return;

	const FIntVector RoofKey = TargetWall->WallKey;
	FString CompName = FString::Printf(TEXT("RoofBlock_%d_%d_%d"), RoofKey.X, RoofKey.Y, RoofKey.Z);
	URoofComp* NewRoof = NewObject<URoofComp>(this, FName(*CompName));

	if (!NewRoof) return;

	NewRoof->Wall = TargetWall;
	NewRoof->FloorKey = TargetWall->FloorKey;
	NewRoof->WallKey = TargetWall->WallKey;
	NewRoof->RoofKey = RoofKey;

	if (RootMesh)
	{
		NewRoof->SetupAttachment(RootMesh);
	}
	else
	{
		NewRoof->SetupAttachment(GetRootComponent());
	}
	NewRoof->RegisterComponent();
	NewRoof->SetWorldScale3D(FVector(1.f, 1.f, 1.f));
	if (RoofMesh)
	{
		NewRoof->SetStaticMesh(RoofMesh);
		NewRoof->SetMaterial(0, RoofMaterial);
	}
	NewRoof->SetSimulatePhysics(false);
	NewRoof->SetMassOverrideInKg(NAME_None, 0.f, true);
	NewRoof->SetCollisionProfileName(TEXT("Building"));
	NewRoof->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	NewRoof->SetCollisionResponseToAllChannels(ECR_Block);
	NewRoof->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	NewRoof->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	FVector WallBoundsMin;
	FVector WallBoundsMax;
	TargetWall->GetLocalBounds(WallBoundsMin, WallBoundsMax);

	FVector RoofBoundsMin;
	FVector RoofBoundsMax;
	NewRoof->GetLocalBounds(RoofBoundsMin, RoofBoundsMax);

	FVector RoofLocalPos = BuildComponent->PreviewMeshComp->GetRelativeLocation();
	RoofLocalPos.Z = TargetWall->GetRelativeLocation().Z + WallBoundsMax.Z - RoofBoundsMin.Z - 15.f;
	NewRoof->SetRelativeLocation(RoofLocalPos);
	NewRoof->SetRelativeRotation(BuildComponent->PreviewMeshComp->GetRelativeRotation());

	TargetWall->Roof = NewRoof;
	RoofMap.Add(RoofKey, NewRoof);
}

// 뗏목의 앵커를 무게중심에서 가장 가까운 메시 -> 바닥 추가 제거할 때만 실행하면 됨
FIntVector ARaftActor::FindDynamicAnchorPoint()
{
	if (!RootMesh || GridMap.Num() == 0) return FIntVector(0, 0, 0);

	// 1. 엔진 내장 함수로 월드 무게중심 획득 후 로컬 변환
	FVector WorldCenterOfMass = RootMesh->GetCenterOfMass();
	FVector LocalCenterOfMass = GetActorTransform().InverseTransformPosition(WorldCenterOfMass);

	// 2. [핵심] 무게중심 좌표를 기반으로 '가상 그리드 중심 인덱스' 바로 계산
	float Size = GridSize;
	int32 CenterX = FMath::RoundToInt(LocalCenterOfMass.X / Size);
	int32 CenterY = FMath::RoundToInt(LocalCenterOfMass.Y / Size);
	int32 CenterZ = 0; // 바닥 레이어 고정

	FIntVector CenterGrid(CenterX, CenterY, CenterZ);

	// 3. 만약 무게중심 칸에 진짜 블록이 딱 있다면? 순회 종료! 바로 리턴 (가장 베스트)
	if (GridMap.Contains(CenterGrid))
	{
		return CenterGrid;
	}

	// 4. 중심 칸이 비어있다면 전체 바닥 중 실제로 가장 가까운 칸을 찾는다.
	FIntVector ClosestGrid = FIntVector(0, 0, 0);
	float ClosestDistanceSq = MAX_FLT;

	for (const auto& Pair : GridMap)
	{
		const FIntVector& CheckGrid = Pair.Key;
		FVector BlockLocalPos(CheckGrid.X * Size, CheckGrid.Y * Size, CheckGrid.Z * Size);
		float DistSq = FVector::DistSquared(LocalCenterOfMass, BlockLocalPos);

		if (DistSq < ClosestDistanceSq)
		{
			ClosestDistanceSq = DistSq;
			ClosestGrid = CheckGrid;
		}
	}

	return ClosestGrid;
}

// 바닥 부쉈을 때 함수 -> 그 바닥에 붙어있는 벽도 부숴지게 추가하기
void ARaftActor::DestroyBlockAndCheckStability(FIntVector TargetGrid)
{
	if (!GridMap.Contains(TargetGrid)) return;

	// 일단 선택한 것부터 파괴
	RemoveFloorAndAttachments(TargetGrid);
	DynamicAnchor = FindDynamicAnchorPoint();
	UE_LOG(LogTemp, Warning, TEXT("블록 파괴됨: (%d, %d, %d)"), TargetGrid.X, TargetGrid.Y, TargetGrid.Z);

	// 파괴된 블록 주변의 사방 4방향 이웃들을 조사 대상으로 선정
	TArray<FIntVector> Directions = {
		FIntVector(1, 0, 0), FIntVector(-1, 0, 0),
		FIntVector(0, 1, 0), FIntVector(0, -1, 0)
	};

	// 연쇄 파괴될 블록들을 중복 없이 모아둘 바구니
	TSet<FIntVector> TotalBlocksToDestroy;

	for (const FIntVector& Dir : Directions)
	{
		FIntVector NeighborGrid = TargetGrid + Dir;

		// 이웃 칸에 블록이 있고, 이미 다른 루프에서 파괴 대상으로 판명된 게 아니라면 연결성 검사 진행
		if (GridMap.Contains(NeighborGrid) && !TotalBlocksToDestroy.Contains(NeighborGrid))
		{
			TSet<FIntVector> IsolatedChunk;
            
			// 중심축(0,0,0)과 연결이 끊어진 고립된 덩어리라면?
			if (!CheckConnectionToAnchor(NeighborGrid, IsolatedChunk))
			{
				// 이번에 찾은 고립된 덩어리 전부를 최종 파괴 바구니에 합칩니다.
				TotalBlocksToDestroy.Append(IsolatedChunk);
			}
		}
	}

	// 4. 고립된 덩어리들 일괄 연쇄 파괴 처리
	for (const FIntVector& GridToDestroy : TotalBlocksToDestroy)
	{
		if (GridMap.Contains(GridToDestroy))
		{
			RemoveFloorAndAttachments(GridToDestroy);
			UE_LOG(LogTemp, Error, TEXT("연쇄 붕괴로 인해 블록 파괴됨: (%d, %d, %d)"), GridToDestroy.X, GridToDestroy.Y, GridToDestroy.Z);
		}
	}
	
	BuoyancyComponent->ComputePontoonsForRaft();
	DynamicAnchor = FindDynamicAnchorPoint();
	UpdateEdgeGridSet();
	
}

// 무게중심에 가까운 메시와 연결되어있는지 확인하는 함수
bool ARaftActor::CheckConnectionToAnchor(FIntVector StartGrid, TSet<FIntVector>& OutIsolatedBlocks)
{
	// 언리얼 표준 TQueue를 이용한 BFS 알고리즘 구현
	TQueue<FIntVector> Queue;
    
	// 방문했던 곳을 다시 가기 않기 위한 체크 테이블 및 결과용 바구니 채우기
	OutIsolatedBlocks.Add(StartGrid);
	Queue.Enqueue(StartGrid);

	bool bIsConnectedToAnchor = false;

	//FIntVector AnchorPoint(0, 0, 0); // 뗏목의 절대 지지대(무게중심 중심축)

	while (!Queue.IsEmpty())
	{
		FIntVector Current;
		Queue.Dequeue(Current);

		// 무게 중심과 가장 가까운 메시의 인덱스를 만나는지
		if (Current == DynamicAnchor)
		{
			bIsConnectedToAnchor = true;
			// 지지대를 찾았더라도 다른 이웃 조사 시 중복 연산을 줄이기 위해 
			// 함수를 바로 끝내지 않고 이 루프 덩어리(OutIsolatedBlocks)를 끝까지 수집하는 것이 좋습니다.
		}

		// 사방 탐색하며 연결된 블록 추적
		TArray<FIntVector> Directions = {
			FIntVector(1, 0, 0), FIntVector(-1, 0, 0),
			FIntVector(0, 1, 0), FIntVector(0, -1, 0)
		};

		for (const FIntVector& Dir : Directions)
		{
			FIntVector NextGrid = Current + Dir;

			// 뗏목에 실제로 존재하는 블록이고, 이번 탐색에서 아직 방문하지 않은 블록이라면
			if (GridMap.Contains(NextGrid) && !OutIsolatedBlocks.Contains(NextGrid))
			{
				OutIsolatedBlocks.Add(NextGrid);
				Queue.Enqueue(NextGrid);
			}
		}
	}

	// 무게중심과 가장 가까운 메시를 만났다면 true(안전), 못 만났다면 false(고립되었으므로 붕괴 대상)를 반환합니다.
	return bIsConnectedToAnchor;
}

void ARaftActor::RemoveFloorAndAttachments(FIntVector TargetGrid)
{
	TArray<FIntVector> WallsToRemove;
	for (const auto& Pair : WallMap)
	{
		if (Pair.Value && Pair.Value->FloorKey == TargetGrid)
		{
			WallsToRemove.Add(Pair.Key);
		}
	}

	for (const FIntVector& WallKey : WallsToRemove)
	{
		RemoveWallAtGrid(WallKey);
	}

	TArray<FIntVector> RoofsToRemove;
	for (const auto& Pair : RoofMap)
	{
		if (Pair.Value && Pair.Value->FloorKey == TargetGrid)
		{
			RoofsToRemove.Add(Pair.Key);
		}
	}

	for (const FIntVector& RoofKey : RoofsToRemove)
	{
		RemoveRoofAtGrid(RoofKey);
	}

	if (UFloorComp** Floor = GridMap.Find(TargetGrid))
	{
		if (*Floor)
		{
			(*Floor)->DestroyComponent();
		}
		GridMap.Remove(TargetGrid);
	}
}

void ARaftActor::RemoveWallAtGrid(FIntVector TargetGrid)
{
	UWallComp** WallPtr = WallMap.Find(TargetGrid);
	if (!WallPtr || !*WallPtr) return;

	UWallComp* Wall = *WallPtr;
	if (Wall->Roof)
	{
		RemoveRoofAtGrid(Wall->Roof->RoofKey);
	}

	if (Wall->Floor)
	{
		Wall->Floor->RemoveWall(Wall);
	}

	Wall->DestroyComponent();
	WallMap.Remove(TargetGrid);
}

void ARaftActor::RemoveRoofAtGrid(FIntVector TargetGrid)
{
	URoofComp** RoofPtr = RoofMap.Find(TargetGrid);
	if (!RoofPtr || !*RoofPtr) return;

	URoofComp* Roof = *RoofPtr;
	if (Roof->Wall && Roof->Wall->Roof == Roof)
	{
		Roof->Wall->Roof = nullptr;
	}

	Roof->DestroyComponent();
	RoofMap.Remove(TargetGrid);
}

void ARaftActor::UpdateEdgeGridSet()
{
	EdgeGridSet.Empty();

	const TArray<FIntVector> Directions = {
		FIntVector(1, 0, 0), FIntVector(-1, 0, 0),
		FIntVector(0, 1, 0), FIntVector(0, -1, 0)
	};

	for (const auto& Pair : GridMap)
	{
		const FIntVector& Grid = Pair.Key;
		bool bIsEdge = false;

		for (const FIntVector& Dir : Directions)
		{
			if (!GridMap.Contains(Grid + Dir))
			{
				bIsEdge = true;
				break;
			}
		}

		if (bIsEdge)
		{
			EdgeGridSet.Add(Grid);
		}
	}
}

bool ARaftActor::GetRandomEdgeFloorForShark(FIntVector& OutGrid, FVector& OutWorldLocation) const
{
	if (EdgeGridSet.Num() == 0)
	{
		return false;
	}

	const int32 TargetIndex = FMath::RandRange(0, EdgeGridSet.Num() - 1);
	int32 CurrentIndex = 0;

	for (const FIntVector& EdgeGrid : EdgeGridSet)
	{
		if (CurrentIndex == TargetIndex)
		{
			OutGrid = EdgeGrid;
			return GetFloorWorldLocation(EdgeGrid, OutWorldLocation);
		}

		++CurrentIndex;
	}

	return false;
}

bool ARaftActor::GetFloorWorldLocation(FIntVector TargetGrid, FVector& OutWorldLocation) const
{
	UFloorComp* const* FloorPtr = GridMap.Find(TargetGrid);
	if (!FloorPtr || !*FloorPtr)
	{
		return false;
	}

	OutWorldLocation = (*FloorPtr)->GetComponentLocation();
	return true;
}

