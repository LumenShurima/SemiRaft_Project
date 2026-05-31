// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildSystem/BuildComponent.h"

#include "BuildSystem/FloorComp.h"
#include "BuildSystem/RaftActor.h"
#include "BuildSystem/RoofComp.h"
#include "BuildSystem/WallComp.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "InventorySystem/InventoryComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UBuildComponent::UBuildComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UBuildComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerRaft = Cast<ARaftActor>(GetOwner());
	if (OwnerRaft)
	{
		UE_LOG(LogTemp, Log, TEXT("OwnerRaft 연결 성공!"));
       
		PreviewMeshComp = NewObject<UStaticMeshComponent>(GetOwner(), TEXT("BuildPreviewMesh"));
		if (PreviewMeshComp)
		{
			PreviewMeshComp->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			PreviewMeshComp->RegisterComponent();
          
			PreviewMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            
			if (FloorMeshAsset) PreviewMeshComp->SetStaticMesh(FloorMeshAsset);
			if (PreviewMaterial) PreviewMeshComp->SetMaterial(0, PreviewMaterial);
            
			PreviewMeshComp->SetVisibility(false);
		}
       
		GridSize = OwnerRaft->GridSize;
	}
	
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (PlayerCharacter)
	{
		PlayerCamera = PlayerCharacter->FindComponentByClass<UCameraComponent>();
		InventoryComp = PlayerCharacter->FindComponentByClass<UInventoryComponent>();
		if (PlayerCamera)
		{
			UE_LOG(LogTemp, Log, TEXT("성공: 플레이어 카메라를 저장했습니다!"));
		}
	}
	
}


// Called every frame
void UBuildComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bBuildModeActive)
	{
		if (OwnerRaft->BlockType == EBlockType::Floor)
		{
			PreviewMeshComp->SetStaticMesh(OwnerRaft->DefaultFloorMesh);
			PreviewMeshComp->SetWorldScale3D(FVector(1.f, 1.f, 0.5f));
			UpdateFloorPreview();
		}
		else if (OwnerRaft->BlockType == EBlockType::Wall)
		{
			PreviewMeshComp->SetStaticMesh(OwnerRaft->WallMesh);
			PreviewMeshComp->SetWorldScale3D(FVector(1.f, 1.f, 1.f));
			UpdateWallPreview();
		}
		else if (OwnerRaft->BlockType == EBlockType::Roof)
		{
			PreviewMeshComp->SetStaticMesh(OwnerRaft->RoofMesh);
			PreviewMeshComp->SetWorldScale3D(FVector(1.f, 1.f, 1.f));
			UpdateRoofPreview();
		}
		
	}
}

void UBuildComponent::UpdateFloorPreview()
{
	if (!PlayerCamera || !OwnerRaft || !PreviewMeshComp) return;
	
	// 카메라 위치에서 라인 시작
	FVector CamLoc = PlayerCamera->GetComponentLocation();
	FVector CamDir = PlayerCamera->GetForwardVector();

	// 라인 끝
	FVector LineEnd = CamLoc + (CamDir * 2000.f);

	// 뗏목의 위치와 벡터
	FVector PlanePoint = OwnerRaft->GetActorLocation();
	FVector PlaneNormal = OwnerRaft->GetActorUpVector();

	// 뗏목 기준 로컬 z가 0인 평면과의 교차점 계산
	FVector IntersectionWorldPoint = FVector::ZeroVector;
	bool bIntersects = FMath::SegmentPlaneIntersection(CamLoc, LineEnd, FPlane(PlanePoint, PlaneNormal), IntersectionWorldPoint);

	if (bIntersects)
	{
		// 교차점을 뗏목 로컬 좌표로 역환산
		FVector LocalIntersection = OwnerRaft->GetActorTransform().InverseTransformPosition(IntersectionWorldPoint);

		// 가상 그리드 인덱스 계산
		float Size = OwnerRaft->GridSize;

		// 가상 그리드
		int32 GridX = FMath::RoundToInt(LocalIntersection.X / Size);
		int32 GridY = FMath::RoundToInt(LocalIntersection.Y / Size);
		int32 GridZ = 0; // 바닥 기준 레이어 0층 고정

		TargetGridCoordinate = FIntVector(GridX, GridY, GridZ);
		
		bool bHasNeighbor = false;
		
		// 이웃
		TArray<FIntVector> NeighborOffsets = {
			FIntVector(1, 0, 0),  // 앞
			FIntVector(-1, 0, 0), // 뒤
			FIntVector(0, 1, 0),  // 오른쪽
			FIntVector(0, -1, 0)  // 왼쪽
		};

		for (const FIntVector& Offset : NeighborOffsets)
		{
			FIntVector CheckGrid = TargetGridCoordinate + Offset;
            
			// OwnerRaft의 가상그리드에 이미 있는지 확인
			if (OwnerRaft && OwnerRaft->GridMap.Contains(CheckGrid))
			{
				bHasNeighbor = true;
				break;
			}
		}

		// 이미 내 자리에 블록이 있거나, 주변에 이웃 블록이 단 하나도 없다면 프리뷰를 숨깁니다.
		if (OwnerRaft->GridMap.Contains(TargetGridCoordinate) || !bHasNeighbor)
		{
			PreviewMeshComp->SetVisibility(false);
			return;
		}
		
		// 해당 그리드의 로컬 좌표
		FVector PreviewLocalPos(
			TargetGridCoordinate.X * Size,
			TargetGridCoordinate.Y * Size,
			0.f
		);
		
		// Z축 두께 보정
		// PreviewLocalPos.Z -= 10.f; 
		
		PreviewMeshComp->SetRelativeLocation(PreviewLocalPos);
		
		// 재료가 인벤토리에 있는지 확인
		TrashItem = InventoryComp->FindItemStacks(FName("Trash"));
		PlasticItem = InventoryComp->FindItemStacks(FName("Plastic"));
		
		if (TrashItem.bSuccess && PlasticItem.bSuccess && TrashItem.FoundCount >= 2 && PlasticItem.FoundCount >= 1)
		{
			PreviewMeshComp->SetMaterial(0, PreviewMaterial);
			bCanBuild = true;
		}else
		{
			//UE_LOG(LogTemp, Warning, );
			PreviewMeshComp->SetMaterial(0, PreviewMaterialRed);
			bCanBuild = false;
		}
		
		PreviewMeshComp->SetVisibility(true);
	}
	else
	{
		PreviewMeshComp->SetVisibility(false);
	}
}

void UBuildComponent::UpdateWallPreview()
{
	if (!PlayerCamera || !OwnerRaft || !PreviewMeshComp) return;

    FVector CamLoc = PlayerCamera->GetComponentLocation();
    FVector CamDir = PlayerCamera->GetForwardVector();
    FVector LineEnd = CamLoc + (CamDir * 2000.f);

    FVector PlanePoint  = OwnerRaft->GetActorLocation();
    FVector PlaneNormal = OwnerRaft->GetActorUpVector();

    FVector IntersectionWorldPoint = FVector::ZeroVector;
    bool bIntersects = FMath::SegmentPlaneIntersection(CamLoc, LineEnd, FPlane(PlanePoint, PlaneNormal), IntersectionWorldPoint);

    if (bIntersects)
    {
        FVector LocalIntersection = OwnerRaft->GetActorTransform().InverseTransformPosition(IntersectionWorldPoint);

        float Size  = OwnerRaft->GridSize;
        int32 GridX = FMath::RoundToInt(LocalIntersection.X / Size);
        int32 GridY = FMath::RoundToInt(LocalIntersection.Y / Size);
        int32 Layer = 0;

        // 교차점 위치에 Floor가 있는지 확인
        FIntVector FloorKey = FIntVector(GridX, GridY, Layer);
        if (!OwnerRaft->GridMap.Contains(FloorKey))
        {	UE_LOG(LogTemp, Log, TEXT("교차점에 Floor 없음!!!!"));
            PreviewMeshComp->SetVisibility(false);
            return;
        }
        else
        {
        	UE_LOG(LogTemp, Log, TEXT("교차점에 Floor 있음!!!!"));
        }

        // Floor 중심으로부터 교차점의 로컬 오프셋으로 방향 결정
        FVector FloorCenter(GridX * Size, GridY * Size, 0.f);
        FVector LocalOffset = LocalIntersection - FloorCenter;

        // |X| vs |Y| 중 큰 쪽 방향이 벽이 붙을 모서리
        int32    WallDir;
        FVector  WallPosOffset;
        FRotator WallRotation;

    	float WallThickness = 10.f;

    	if (FMath::Abs(LocalOffset.X) >= FMath::Abs(LocalOffset.Y))
    	{
    		if (LocalOffset.X >= 0)
    		{
    			WallDir       = 0;
    			WallPosOffset = FVector(Size * 0.5f - WallThickness, 0.f, 0.f); // 안쪽으로
    			WallRotation  = FRotator(0.f, 0.f, 0.f);
    		}
    		else
    		{
    			WallDir       = 1;
    			WallPosOffset = FVector(-Size * 0.5f + WallThickness, 0.f, 0.f); // 안쪽으로
    			WallRotation  = FRotator(0.f, 180.f, 0.f);
    		}
    	}
    	else
    	{
    		if (LocalOffset.Y >= 0)
    		{
    			WallDir       = 2;
    			WallPosOffset = FVector(0.f, Size * 0.5f - WallThickness, 0.f); // 안쪽으로
    			WallRotation  = FRotator(0.f, 90.f, 0.f);
    		}
    		else
    		{
    			WallDir       = 3;
    			WallPosOffset = FVector(0.f, -Size * 0.5f + WallThickness, 0.f); // 안쪽으로
    			WallRotation  = FRotator(0.f, -90.f, 0.f);
    		}
    	}

        // WallMap에 이미 벽이 있는지 확인
        TargetGridCoordinate = FIntVector(GridX, GridY, Layer * 10 + WallDir);
        if (OwnerRaft->WallMap.Contains(TargetGridCoordinate))
        {
            PreviewMeshComp->SetVisibility(false);
            return;
        }

        FVector WallBoundsMin;
        FVector WallBoundsMax;
        PreviewMeshComp->GetLocalBounds(WallBoundsMin, WallBoundsMax);

        // 프리뷰 위치/회전 설정
        FVector WallPreviewLocalPos = FloorCenter + WallPosOffset;
        WallPreviewLocalPos.Z = Size * 0.25f - WallBoundsMin.Z;

        PreviewMeshComp->SetRelativeLocation(WallPreviewLocalPos);
        PreviewMeshComp->SetRelativeRotation(WallRotation);

        // 재료 확인
        TrashItem   = InventoryComp->FindItemStacks(FName("Trash"));
        PlasticItem = InventoryComp->FindItemStacks(FName("Plastic"));

        if (TrashItem.bSuccess && PlasticItem.bSuccess && TrashItem.FoundCount >= 2 && PlasticItem.FoundCount >= 1)
        {
            PreviewMeshComp->SetMaterial(0, PreviewMaterial);
            bCanBuild = true;
        }
        else
        {
            PreviewMeshComp->SetMaterial(0, PreviewMaterialRed);
            bCanBuild = false;
        }

        PreviewMeshComp->SetVisibility(true);
    }
    else
    {
        PreviewMeshComp->SetVisibility(false);
    }

}

void UBuildComponent::UpdateRoofPreview()
{
	if (!PlayerCamera || !OwnerRaft || !PreviewMeshComp) return;
	TargetWallComp = nullptr;

    FVector CamLoc = PlayerCamera->GetComponentLocation();
    FVector CamDir = PlayerCamera->GetForwardVector();
    FVector LineEnd = CamLoc + (CamDir * 2000.f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    Params.AddIgnoredActor(PlayerCharacter);
    Params.AddIgnoredComponent(PreviewMeshComp.Get());
    Params.AddIgnoredComponent(Cast<UPrimitiveComponent>(OwnerRaft->RootMesh));

    // 맞은 게 WallComp인지 확인
    UWallComp* HitWall = nullptr;
    if (GetWorld()->LineTraceSingleByChannel(HitResult, CamLoc, LineEnd, ECC_Visibility, Params))
    {
        HitWall = Cast<UWallComp>(HitResult.GetComponent());
    }

    if (!HitWall)
    {
        float BestScore = TNumericLimits<float>::Max();

        for (const auto& Pair : OwnerRaft->WallMap)
        {
            UWallComp* Wall = Pair.Value;
            if (!Wall || Wall->Roof || OwnerRaft->RoofMap.Contains(Wall->WallKey))
            {
                continue;
            }

            FVector WallBoundsMin;
            FVector WallBoundsMax;
            Wall->GetLocalBounds(WallBoundsMin, WallBoundsMax);

            const float PickRadius = (WallBoundsMax - WallBoundsMin).Size() * 0.5f + 60.f;
            const FVector WallWorldLocation = Wall->GetComponentLocation();
            const FVector ClosestPoint = FMath::ClosestPointOnSegment(WallWorldLocation, CamLoc, LineEnd);
            const float DistanceSq = FVector::DistSquared(WallWorldLocation, ClosestPoint);

            if (DistanceSq <= FMath::Square(PickRadius) && DistanceSq < BestScore)
            {
                BestScore = DistanceSq;
                HitWall = Wall;
            }
        }

        if (!HitWall)
        {
            PreviewMeshComp->SetVisibility(false);
            return;
        }
    }

    if (HitWall->Roof || OwnerRaft->RoofMap.Contains(HitWall->WallKey))
    {
        PreviewMeshComp->SetVisibility(false);
        return;
    }

	TargetWallComp = HitWall;

    // 지붕은 벽 기준으로 하나만 설치한다.
    FIntVector RoofKey = HitWall->WallKey;
    TargetGridCoordinate = RoofKey;

    // 프리뷰 위치/회전 설정
    FVector WallBoundsMin;
    FVector WallBoundsMax;
    HitWall->GetLocalBounds(WallBoundsMin, WallBoundsMax);

    FVector RoofBoundsMin;
    FVector RoofBoundsMax;
    PreviewMeshComp->GetLocalBounds(RoofBoundsMin, RoofBoundsMax);

    FVector RoofLocalPos = HitWall->GetRelativeLocation();
    RoofLocalPos.Z += WallBoundsMax.Z - RoofBoundsMin.Z - 15.f;

    FRotator RoofRotation = HitWall->GetRelativeRotation();
    if (bRoofFlipped)
    {
        RoofRotation.Yaw += 180.f;
    }

    PreviewMeshComp->SetRelativeLocation(RoofLocalPos);
    PreviewMeshComp->SetRelativeRotation(RoofRotation);

    // 재료 확인
    TrashItem   = InventoryComp->FindItemStacks(FName("Trash"));
    PlasticItem = InventoryComp->FindItemStacks(FName("Plastic"));

    if (TrashItem.bSuccess && PlasticItem.bSuccess
        && TrashItem.FoundCount >= 2 && PlasticItem.FoundCount >= 1)
    {
        PreviewMeshComp->SetMaterial(0, PreviewMaterial);
        bCanBuild = true;
    }
    else
    {
        PreviewMeshComp->SetMaterial(0, PreviewMaterialRed);
        bCanBuild = false;
    }

    PreviewMeshComp->SetVisibility(true);
}

void UBuildComponent::BuildFloor()
{
	// 망치 좌클릭 시 호출될 함수
	if (!OwnerRaft || !PreviewMeshComp->IsVisible()) return;

	// 이미 그 가상 그리드 주소에 바닥이 깔려있다면 중복 설치 차단
	if (OwnerRaft->GridMap.Contains(TargetGridCoordinate)) return;

	// 그 위치에 스폰
	if (bCanBuild)
	{
		bool result1 = InventoryComp->ItemConsumption(TrashItem, 2);
		bool result2 = InventoryComp->ItemConsumption(PlasticItem, 1);
		if (result1 && result2)
		{
			OwnerRaft->SpawnFloorAtGrid(TargetGridCoordinate);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("bCanBuild가 false"));
	}
}

void UBuildComponent::BuildWall()
{
	// 망치 좌클릭 시 호출될 함수
	if (!OwnerRaft || !PreviewMeshComp->IsVisible()) return;

	// 이미 그 가상 그리드 주소에 벽이 깔려있다면 중복 설치 차단
	if (OwnerRaft->WallMap.Contains(TargetGridCoordinate)) return;

	// 그 위치에 스폰
	if (bCanBuild)
	{
		bool result1 = InventoryComp->ItemConsumption(TrashItem, 2);
		bool result2 = InventoryComp->ItemConsumption(PlasticItem, 1);
		if (result1 && result2)
		{
			OwnerRaft->SpawnWallAtGrid(TargetGridCoordinate);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("bCanBuild가 false"));
	}
}

void UBuildComponent::BuildRoof()
{
	if (!OwnerRaft || !PreviewMeshComp->IsVisible() || !TargetWallComp) return;

	if (TargetWallComp->Roof || OwnerRaft->RoofMap.Contains(TargetWallComp->WallKey)) return;

	if (bCanBuild)
	{
		bool result1 = InventoryComp->ItemConsumption(TrashItem, 2);
		bool result2 = InventoryComp->ItemConsumption(PlasticItem, 1);
		if (result1 && result2)
		{
			OwnerRaft->SpawnRoofAtGrid(TargetWallComp->WallKey, TargetWallComp);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("bCanBuild가 false"));
	}
}


void UBuildComponent::SetBuildModeActive(bool bActive)
{
	bBuildModeActive = bActive;

	// 빌드 모드 off 프리뷰 메쉬도 같이 숨김
	if (!bBuildModeActive && PreviewMeshComp)
	{
		PreviewMeshComp->SetVisibility(false);
	}
}

void UBuildComponent::SetDestroyModeActive(bool bActive)
{
	bDestroyModeActive = bActive;
}

void UBuildComponent::DetectDestroyMesh()
{
	if (!PlayerCamera || !OwnerRaft) return;

	FVector CamLoc = PlayerCamera->GetComponentLocation();
	FVector CamDir = PlayerCamera->GetForwardVector();
	FVector LineEnd = CamLoc + (CamDir * 1000.f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	Params.AddIgnoredActor(PlayerCharacter);
	Params.AddIgnoredComponent(Cast<UPrimitiveComponent>(OwnerRaft->RootMesh));
	Params.AddIgnoredComponent(PreviewMeshComp.Get());

	TargetMeshComp = nullptr;

	TArray<FHitResult> HitResults;
	if (GetWorld()->LineTraceMultiByChannel(HitResults, CamLoc, LineEnd, ECC_Visibility, Params))
	{
		for (const FHitResult& Hit : HitResults)
		{
			UStaticMeshComponent* HitMeshComp = Cast<UStaticMeshComponent>(Hit.GetComponent());
			if (!HitMeshComp)
			{
				continue;
			}

			if (HitMeshComp->IsA(UFloorComp::StaticClass()) ||
				HitMeshComp->IsA(UWallComp::StaticClass()) ||
				HitMeshComp->IsA(URoofComp::StaticClass()))
			{
				TargetMeshComp = HitMeshComp;
				return;
			}
		}
	}
}

void UBuildComponent::DestroyTargetMeshComp(UStaticMeshComponent* meshcomp)
{
	if (!meshcomp) return;

	if (UFloorComp* Floor = Cast<UFloorComp>(meshcomp))
	{
		OwnerRaft->DestroyBlockAndCheckStability(Floor->GridKey);
	}
	else if (UWallComp* Wall = Cast<UWallComp>(meshcomp))
	{
		OwnerRaft->RemoveWallAtGrid(Wall->WallKey);
	}
	else if (URoofComp* Roof = Cast<URoofComp>(meshcomp))
	{
		OwnerRaft->RemoveRoofAtGrid(Roof->RoofKey);
	}
	else
	{
		return;
	}
}

