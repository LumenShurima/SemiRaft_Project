// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuoyancyTypes.h"
#include "RaftSystem/RaftSystemStruct.h"
#include "BuoyancyRootActor.generated.h"

class UExtendedBuoyancyComponent;
class URaftSystemStaticMeshComponent;

UENUM()
enum class EFloorDirection : uint8
{
	East,
	West,
	South,
	North
};


USTRUCT()
struct FBoundingBoxPoint
{
	GENERATED_BODY()
	
	UPROPERTY()
	double MinX;
	
	UPROPERTY()
	double MaxX;
	
	UPROPERTY()
	double MinY;
	
	UPROPERTY()
	double MaxY;
};


UCLASS()
class SEMIRAFT_API ABuoyancyRootActor : public AActor
{
	GENERATED_BODY()
		
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> RootMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UExtendedBuoyancyComponent> BuoyancyComponent = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GridSize = 500.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BuoyancyCoefficientBias = 0.4f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* FloorMesh;
	
	UPROPERTY(EditAnywhere, Category="Physics Safety")
	float MaxLinearSpeed = 1200.0f; // cm/s

	UPROPERTY(EditAnywhere, Category="Physics Safety")
	float MaxUpwardSpeed = 600.0f; // cm/s

	UPROPERTY(EditAnywhere, Category="Physics Safety")
	float MaxDownwardSpeed = 1200.0f; // cm/s

	UPROPERTY(EditAnywhere, Category="Physics Safety")
	float MaxAngularSpeedDeg = 120.0f; // deg/s
	
	FBoundingBoxPoint Bounds;
	
	UPROPERTY()
	FVector PrevLinearSpeed = FVector::ZeroVector;
	
	UPROPERTY()
	FVector PrevAngularDeg = FVector::ZeroVector;

private:
	UPROPERTY(Transient, DuplicateTransient)
	TMap<TObjectPtr<UStaticMeshComponent>, FIntPoint> FloorToGridMap;
	
	UPROPERTY(Transient, DuplicateTransient)
	TMap<FIntPoint, TObjectPtr<UStaticMeshComponent>> GridToFloorMap;

public:
	FORCEINLINE TMap<TObjectPtr<UStaticMeshComponent>, FIntPoint>& GetFloorToGridMap() { return FloorToGridMap; }
	FORCEINLINE TMap<FIntPoint, TObjectPtr<UStaticMeshComponent>>&  GetGridToFloorMap() { return GridToFloorMap; }
	
	

	

	
	
public:	
	// Sets default values for this actor's properties
	ABuoyancyRootActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UStaticMeshComponent* CreateFloorComponent(const FIntPoint& Grid);

	UFUNCTION(BlueprintCallable)
	UStaticMeshComponent* AddFloor(UStaticMeshComponent* TargetFloor, EFloorDirection Direction);
	
	UFUNCTION(BlueprintCallable)
	void RemoveFloor(UStaticMeshComponent* TargetFloor);
	void RebuildBuoyancyPontoons();

};

UCLASS()
class SEMIRAFT_API AFloorActor : public AActor
{
	GENERATED_BODY()
	
};
