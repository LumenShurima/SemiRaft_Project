// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildSystem/RaftActor.h"

#include "BuildSystem/BuildComponent.h"
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
		
		UStaticMeshComponent* NewFloorComp = CreateDefaultSubobject<UStaticMeshComponent>(FName(*UniqueCompName));
        
		if (NewFloorComp)
		{
			NewFloorComp->SetupAttachment(RootComponent);
			
			if (DefaultFloorMesh)
			{
				NewFloorComp->SetStaticMesh(DefaultFloorMesh);
			}
			NewFloorComp->SetCollisionProfileName(TEXT("Building"));
			
			FVector RelativePosition(Coord.X * GridSize, Coord.Y * GridSize, Coord.Z * GridSize);
			NewFloorComp->SetRelativeLocation(RelativePosition);
			
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
}

void ARaftActor::SpawnFloorAtGrid(FIntVector TargetGrid)
{
	FString CompName = FString::Printf(TEXT("FloorBlock_%d_%d_%d"), TargetGrid.X, TargetGrid.Y, TargetGrid.Z);
	UStaticMeshComponent* NewFloor = NewObject<UStaticMeshComponent>(this, FName(*CompName));
	NewFloor->SetWorldScale3D(FVector(1.f, 1.f, 1.0f));
    
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
		NewFloor->SetWorldScale3D(FVector(1.f, 1.f, 1.0f));
        
		if (DefaultFloorMesh) 
		{
			NewFloor->SetStaticMesh(DefaultFloorMesh);
		}
		NewFloor->SetCollisionProfileName(TEXT("BlockAll"));
		
		float Size = GridSize; 
		FVector FloorLocalPos(
			TargetGrid.X * Size,
			TargetGrid.Y * Size,
			0.f // 바닥 레이어는 평면 고정
		);
        
		// 로컬 좌표 설정
		NewFloor->SetRelativeLocation(FloorLocalPos);

		// 주소록에 등록
		GridMap.Add(TargetGrid, NewFloor);
	}
	BuoyancyComponent->ComputePontoonsForRaft();
	DynamicAnchor = FindDynamicAnchorPoint();
	
	// 외곽 뗏목 계산
	UpdateEdgeOnSpawn(TargetGrid);
}

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

	// 4. 만약 중심 칸이 비어있다면, 주변 반경 1~2칸 레이어만 확장하며 검사 (최대 9~25번만 루프)
	FIntVector ClosestGrid = FIntVector(0, 0, 0);
	float ClosestDistanceSq = MAX_FLT;
	bool bFound = false;

	// 반경 3칸까지 검사
	int32 SearchRadius = 3; 

	for (int32 x = -SearchRadius; x <= SearchRadius; ++x)
	{
		for (int32 y = -SearchRadius; y <= SearchRadius; ++y)
		{
			FIntVector CheckGrid(CenterX + x, CenterY + y, CenterZ);

			// 전체 맵을 도는 게 아니라, 가상 주소록에 이 근처 칸이 있는지만 콕 찝어 검사!
			if (GridMap.Contains(CheckGrid))
			{
				FVector BlockLocalPos(CheckGrid.X * Size, CheckGrid.Y * Size, CheckGrid.Z * Size);
				float DistSq = FVector::DistSquared(LocalCenterOfMass, BlockLocalPos);

				if (DistSq < ClosestDistanceSq)
				{
					ClosestDistanceSq = DistSq;
					ClosestGrid = CheckGrid;
					bFound = true;
				}
			}
		}
	}

	// 안전장치: 혹시라도 파괴 연출 등으로 순간적으로 반경 내에 블록이 없다면 구관이 명관, (0,0,0)을 뱉거나
	// GridMap의 Begin 포인터 하나를 안전빵으로 리턴합니다.
	if (!bFound)
	{
		for (const auto& Pair : GridMap) return Pair.Key;
	}

	return ClosestGrid;
}

void ARaftActor::DestroyBlockAndCheckStability(FIntVector TargetGrid)
{
	if (!GridMap.Contains(TargetGrid)) return;

	// 일단 선택한 것부터 파괴
	if (GridMap[TargetGrid])
	{
		GridMap[TargetGrid]->DestroyComponent();
	}
	GridMap.Remove(TargetGrid);
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
		if (GridMap.Contains(GridToDestroy) && GridMap[GridToDestroy])
		{
			GridMap[GridToDestroy]->DestroyComponent();
			GridMap.Remove(GridToDestroy);
			UE_LOG(LogTemp, Error, TEXT("연쇄 붕괴로 인해 블록 파괴됨: (%d, %d, %d)"), GridToDestroy.X, GridToDestroy.Y, GridToDestroy.Z);
		}
	}
	
	BuoyancyComponent->ComputePontoonsForRaft();
	
	// 외곽 뗏목 계산
	UpdateEdgeOnDestroy(TargetGrid);
}

// 뗏목의 앵커를 무게중심에서 가장 가까운 메시로 다시 수정하기
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

bool ARaftActor::IsEdgeBlock(FIntVector TargetGrid)
{
	// 사방(이웃)을 검사할 상대 좌표 배열 (Z축은 평면이므로 X, Y만)
	TArray<FIntVector> Neighbors = {
		FIntVector(1, 0, 0),  // 우
		FIntVector(-1, 0, 0), // 좌
		FIntVector(0, 1, 0),  // 앞
		FIntVector(0, -1, 0)  // 뒤
	};

	for (const FIntVector& Dir : Neighbors)
	{
		FIntVector CheckPos = TargetGrid + Dir;
        
		// 사방 중 단 한 곳이라도 GridMap에 없다면? 물과 닿은 외곽이다!
		if (!GridMap.Contains(CheckPos))
		{
			return true; 
		}
	}

	// 사방이 전부 블록으로 꽉 차 있다면 내부 블록이다.
	return false;
}

void ARaftActor::UpdateEdgeOnSpawn(FIntVector NewGrid)
{
	// 1. 새로 지어진 블록은 무조건 외곽 리스트에 추가
	EdgeGridSet.Add(NewGrid);

	// 2. 내 주변 사방 블록들을 다시 검사해서 내부로 숨겨졌으면 외곽에서 탈락시킴
	TArray<FIntVector> Neighbors = { FIntVector(1,0,0), FIntVector(-1,0,0), FIntVector(0,1,0), FIntVector(0,-1,0) };
	for (const FIntVector& Dir : Neighbors)
	{
		FIntVector NeighborGrid = NewGrid + Dir;
		if (GridMap.Contains(NeighborGrid))
		{
			// 주변 블록이 나 때문에 사방이 막혔다면 외곽 세트에서 제거
			if (!IsEdgeBlock(NeighborGrid))
			{
				EdgeGridSet.Remove(NeighborGrid);
			}
		}
	}
}

void ARaftActor::UpdateEdgeOnDestroy(FIntVector DestroyedGrid)
{
	// 1. 파괴된 블록은 외곽 리스트에서 무조건 제거
	EdgeGridSet.Remove(DestroyedGrid);

	// 2. 내 주변 사방 블록들은 이제 바다와 맞닿게 되므로 외곽 리스트에 추가 가능성 검사
	TArray<FIntVector> Neighbors = { FIntVector(1,0,0), FIntVector(-1,0,0), FIntVector(0,1,0), FIntVector(0,-1,0) };
	for (const FIntVector& Dir : Neighbors)
	{
		FIntVector NeighborGrid = DestroyedGrid + Dir;
        
		// 주변에 살아있는 블록이 있다면 이제 외곽이 되었을 확률이 100%
		if (GridMap.Contains(NeighborGrid))
		{
			EdgeGridSet.Add(NeighborGrid);
		}
	}
}

