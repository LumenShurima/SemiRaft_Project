// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WaterBodyOceanActor.h"
#include "WeatherWaterBodyOcean.generated.h"


class UWaterWavesBase;
class UGerstnerWaterWaveGeneratorSimple;

USTRUCT()
struct FWeatherGerstnerWaveGeneratorSnapshot
{
	GENERATED_BODY()
	
	int NumWaves = 16;
	int Seed = 0;
	float Randomness = 0.0f;

	float MinWavelength = 521.0f;
	float MaxWavelength = 6000.0f;
	float WavelengthFalloff = 2.0f;

	float MinAmplitude = 4.0f;
	float MaxAmplitude = 80.0f;
	float AmplitudeFalloff = 2.0f;

	float WindAngleDeg = 0.0f;
	float DirectionAngularSpreadDeg = 1325.0f;

	float SmallWaveSteepness = 0.4f;
	float LargeWaveSteepness = 0.2f;
	float SteepnessFalloff = 1.0f;
};
/**
 * 
 */
UCLASS()
class SEMIRAFT_API AWeatherWaterBodyOcean : public AWaterBodyOcean
{
	GENERATED_UCLASS_BODY()

public:
	virtual void PostLoad() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UWaterWavesAsset*> WaterWavesArray;
	
	virtual void Tick(float DeltaSeconds) override;
	
	UFUNCTION()
	void WaveToTargetWave(int TargetIdx, float InLerpTime);
	
private:
	UPROPERTY()
	FWeatherGerstnerWaveGeneratorSnapshot CurrentSnapShot;
	
	UPROPERTY()
	FWeatherGerstnerWaveGeneratorSnapshot TargetSnapShot;
	
	UPROPERTY()
	bool bWaveToTargetWave = false;
	
	float WaveLerpElapsedTime = 0.0f;
	float WaveLerpDuration = 1.0f;
	
	UPROPERTY()
	TObjectPtr<UGerstnerWaterWaveGeneratorSimple> CurrentWaterWaveGenerator;
	
	UFUNCTION()
	UGerstnerWaterWaveGeneratorSimple* GetWaterWaveGenerator(UWaterWavesBase* InWaterWaves);
	


#if WITH_EDITOR
	virtual void PostActorCreated() override;
#endif

private:
	void ApplyDefaultOceanSettings();
};
