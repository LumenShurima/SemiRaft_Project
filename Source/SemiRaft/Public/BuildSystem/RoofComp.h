// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "RoofComp.generated.h"

/**
 * 
 */
UCLASS()
class SEMIRAFT_API URoofComp : public UStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	URoofComp();

	UPROPERTY()
	class UWallComp* Wall = nullptr;

	UPROPERTY()
	FIntVector FloorKey = FIntVector::ZeroValue;

	UPROPERTY()
	FIntVector WallKey = FIntVector::ZeroValue;

	UPROPERTY()
	FIntVector RoofKey = FIntVector::ZeroValue;
};
