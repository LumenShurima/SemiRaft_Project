// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "WallComp.generated.h"

/**
 * 
 */
UCLASS()
class SEMIRAFT_API UWallComp : public UStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	UWallComp();

	UPROPERTY()
	class URoofComp* Roof = nullptr;
	
	UPROPERTY()
	class UFloorComp* Floor;

	UPROPERTY()
	FIntVector FloorKey = FIntVector::ZeroValue;

	UPROPERTY()
	FIntVector WallKey = FIntVector::ZeroValue;
	
	void DestroyRoof();
};
