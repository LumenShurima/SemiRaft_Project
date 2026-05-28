// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WaterWaves.h"
#include "WaterWavesAssetController.generated.h"


class UGerstnerWaterWaves;

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class SEMIRAFT_API UWaterWavesAssetController : public UWaterWavesAssetReference
{
	GENERATED_BODY()
	
	
	UWaterWavesAssetController();
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<UWaterWavesAsset*> WaterWaveSources;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UGerstnerWaterWaves> CurrentWave;
	
	void WaveToWave(int TargetWaveIdx, float LerpTime);
	
};
