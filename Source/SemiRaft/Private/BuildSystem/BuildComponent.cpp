// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildSystem/BuildComponent.h"

#include "BuildSystem/RaftActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
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
		UpdateBuildPreview();
	}
}

void UBuildComponent::UpdateBuildPreview()
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
		PreviewMeshComp->SetVisibility(true);
	}
	else
	{
		PreviewMeshComp->SetVisibility(false);
	}
}

void UBuildComponent::BuildFloor()
{
	// 망치 좌클릭 시 호출될 함수
	if (!OwnerRaft || !PreviewMeshComp->IsVisible()) return;

	// 이미 그 가상 그리드 주소에 바닥이 깔려있다면 중복 설치 차단
	if (OwnerRaft->GridMap.Contains(TargetGridCoordinate)) return;

	// 그 위치에 스폰
	OwnerRaft->SpawnFloorAtGrid(TargetGridCoordinate);
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

