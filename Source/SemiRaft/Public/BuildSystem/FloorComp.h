// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "FloorComp.generated.h"

/**
 * 
 */
UCLASS()
class SEMIRAFT_API UFloorComp : public UStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	UFloorComp();

	UPROPERTY()
	FIntVector GridKey = FIntVector::ZeroValue;
	
	UPROPERTY()
	TArray<class UWallComp*> Walls;
	
	void AddWalls(UWallComp* wall);
	
	void AllWallsDestroy();
	
	void RemoveWall(UWallComp* wall);
};
