// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeatherManager.generated.h"

class AWeatherVolumetricCloud;
class AWeatherWaterBodyOcean;
class UDirectionalLightComponent;
class UExponentialHeightFogComponent;
class USkyAtmosphereComponent;
class USkyLightComponent;
class UPostProcessComponent;

UCLASS()
class SEMIRAFT_API AWeatherManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeatherManager();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AWeatherVolumetricCloud> VolumetricCloud;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AWeatherWaterBodyOcean> WaterBodyOcean;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UDirectionalLightComponent> DirectionalLight;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UExponentialHeightFogComponent> ExponentialHeightFog;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UPostProcessComponent> PostProcess;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USkyLightComponent> SkyLight;
	
	


public:	
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable)
	void RunTestWaterBodyFunction(int TargetWaveIdx, float LerpDuration);

};
