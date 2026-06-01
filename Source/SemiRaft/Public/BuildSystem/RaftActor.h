// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RaftActor.generated.h"

UENUM()
enum class EBlockType : uint8
{
	Floor,
	Wall,
	Roof
};

UCLASS()
class SEMIRAFT_API ARaftActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARaftActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<class UFloorComp> RootMesh;
	
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "MyVar")
	UStaticMesh* DefaultFloorMesh;
	
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "MyVar")
	UStaticMesh* WallMesh;
	
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "MyVar")
	UStaticMesh* RoofMesh;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere,Category = "MyVar")
	TMap<FIntVector, UFloorComp*> GridMap;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere,Category = "MyVar")
	TMap<FIntVector, class UWallComp*> WallMap;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere,Category = "MyVar")
	TMap<FIntVector, class URoofComp*> RoofMap;
	
	UPROPERTY()
	TSet<FIntVector> EdgeGridSet; // 상어의 표적이 될 뗏목 외곽
	
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "MyVar")
	class UBuildComponent* BuildComponent;
	
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "MyVar")
	TObjectPtr<class URaftPlatformBuoyancyComponent> BuoyancyComponent;
	
	UPROPERTY()
	FIntVector DynamicAnchor;
	
	UPROPERTY()
	float GridSize;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere,Category = "MyVar")
	EBlockType BlockType = EBlockType::Floor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Build")
	UMaterialInterface* FloorMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Build")
	UMaterialInterface* WallMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Build")
	UMaterialInterface* RoofMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine")
	TSubclassOf<AActor> EngineClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Engine")
	TObjectPtr<AActor> InstalledEngine = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine")
	float BaseForceAmount = 100000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine")
	float EngineForceAmount = 200000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine")
	float ForceRandomRange = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine")
	float EngineBackOffset = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Engine")
	float EngineUpOffset = 30.f;
	
	void SpawnFloorAtGrid(FIntVector TargetGrid);
	void SpawnWallAtGrid(FIntVector TargetGrid);
	void SpawnRoofAtGrid(FIntVector TargetGrid, class UWallComp* TargetWall);
	UFUNCTION(BlueprintCallable, Category="Engine")
	bool InstallEngine();
	FIntVector FindDynamicAnchorPoint();
	void DestroyBlockAndCheckStability(FIntVector TargetGrid);
	bool CheckConnectionToAnchor(FIntVector StartGrid, TSet<FIntVector>& OutIsolatedBlocks);
	void RemoveFloorAndAttachments(FIntVector TargetGrid);
	void RemoveWallAtGrid(FIntVector TargetGrid);
	void RemoveRoofAtGrid(FIntVector TargetGrid);
	void UpdateEdgeGridSet();

	UFUNCTION(BlueprintCallable, Category = "Raft|Shark")
	bool GetRandomEdgeFloorForShark(FIntVector& OutGrid, FVector& OutWorldLocation) const;

	UFUNCTION(BlueprintCallable, Category = "Raft|Shark")
	bool GetFloorWorldLocation(FIntVector TargetGrid, FVector& OutWorldLocation) const;
};
