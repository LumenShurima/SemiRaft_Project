// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuildComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SEMIRAFT_API UBuildComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UBuildComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	// 내가 타고 있는 뗏목 액터 참조 포인터
	UPROPERTY(EditAnywhere, Category = "MyVar")
	TObjectPtr<class ARaftActor> OwnerRaft;

	// 빌드 모드가 켜져있는지 확인하는 플래그
	UPROPERTY(EditAnywhere, Category = "MyVar")
	bool bBuildModeActive;

	// 현재 마우스가 가리키고 있는 가상 그리드 주소 (예: 1, 0, 0)
	FIntVector TargetGridCoordinate;

	// 실시간으로 마우스 따라다닐 미리보기용 컴포넌트
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> PreviewMeshComp;

	// 에디터에서 지정할 바닥 메쉬 에셋
	UPROPERTY(EditDefaultsOnly, Category = "MyVar")
	TObjectPtr<UStaticMesh> FloorMeshAsset;

	// 에디터에서 지정할 미리보기용 반투명 머터리얼 에셋
	UPROPERTY(EditDefaultsOnly, Category = "MyVar")
	TObjectPtr<UMaterialInterface> PreviewMaterial;
	
	UPROPERTY(EditDefaultsOnly, Category = "MyVar")
	TObjectPtr<UMaterialInterface> PreviewMaterialRed;
	
	float GridSize;

	// 실시간 프리뷰를 업데이트하는 함수 (틱에서 호출)
	void UpdateBuildPreview();

	// 마우스 좌클릭 시 실제로 뗏목에 바닥을 스폰할 함수
	void BuildFloor();
	
	UPROPERTY()
	TObjectPtr<class UCameraComponent> PlayerCamera;
	
	UFUNCTION(BlueprintCallable)
	void SetBuildModeActive(bool bActive);
};
