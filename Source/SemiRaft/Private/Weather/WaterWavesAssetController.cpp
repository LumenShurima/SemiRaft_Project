// Fill out your copyright notice in the Description page of Project Settings.


#include "Weather/WaterWavesAssetController.h"

#include "GerstnerWaterWaves.h"

UWaterWavesAssetController::UWaterWavesAssetController()
{
	
}

void UWaterWavesAssetController::WaveToWave(int TargetWaveIdx, float LerpTime)
{
	CurrentWave = Cast<UGerstnerWaterWaves>(GetWaterWaves());
	if (!IsValid(CurrentWave))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%s: Current Wave is Not Valid."),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
	}
}
