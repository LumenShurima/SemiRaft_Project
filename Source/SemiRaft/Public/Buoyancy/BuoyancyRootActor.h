// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuoyancyTypes.h"
#include "BuoyancyRootActor.generated.h"

class UBuoyancyComponent;

UENUM()
enum class EFloorDirection : uint8
{
	East,
	West,
	South,
	North
};



UCLASS()
class SEMIRAFT_API ABuoyancyRootActor : public AActor
{
	GENERATED_BODY()
		
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> RootMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBuoyancyComponent> BuoyancyComponent = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GridSize;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* FloorMesh;
	
private:
	
	
	UPROPERTY()
	TMap<UStaticMeshComponent*, FIntPoint> FloorToGridMap;
	
	UPROPERTY()
	TMap<FIntPoint, UStaticMeshComponent*> GridToFloorMap;
	
	TSet<FIntPoint> GridToPontoonMap;
	

	

	
	
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
	void RemoveFloor(UStaticMeshComponent* TargetFloor);
	void RebuildBuoyancyPontoons();
	void AddPontoonUnique(TSet<FIntPoint>& PontoonSet, const FIntPoint& PontoonGrid);


};
