// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RaftActor.generated.h"

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
	TObjectPtr<UStaticMeshComponent> RootMesh;
	
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "MyVar")
	UStaticMesh* DefaultFloorMesh;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere,Category = "MyVar")
	TMap<FIntVector, UStaticMeshComponent*> GridMap;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MyVar")
	TSet<FIntVector> EdgeGridSet; // 상어의 표적이 될 뗏목 외곽
	
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "MyVar")
	class UBuildComponent* BuildComponent;
	
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "MyVar")
	TObjectPtr<class URaftPlatformBuoyancyComponent> BuoyancyComponent;
	
	UPROPERTY()
	FIntVector DynamicAnchor;
	
	UPROPERTY()
	float GridSize;
	
	void SpawnFloorAtGrid(FIntVector TargetGrid);
	FIntVector FindDynamicAnchorPoint();
	UFUNCTION(BlueprintCallable)
	void DestroyBlockAndCheckStability(FIntVector TargetGrid);
	bool CheckConnectionToAnchor(FIntVector StartGrid, TSet<FIntVector>& OutIsolatedBlocks);
	bool IsEdgeBlock(FIntVector TargetGrid);
	void UpdateEdgeOnSpawn(FIntVector NewGrid);
	void UpdateEdgeOnDestroy(FIntVector DestroyedGrid);
};
