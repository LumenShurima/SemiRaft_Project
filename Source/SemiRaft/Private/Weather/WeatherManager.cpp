// Fill out your copyright notice in the Description page of Project Settings.


#include "Weather/WeatherWaterBodyOcean.h"
#include "Weather/WeatherVolumetricCloud.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/PostProcessComponent.h"
#include "Weather/WeatherManager.h"

#include "NavigationSystemTypes.h"
#include "ShaderPrintParameters.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AWeatherManager::AWeatherManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	DirectionalLight		= CreateDefaultSubobject<UDirectionalLightComponent>		("DirectionalLight");
	ExponentialHeightFog	= CreateDefaultSubobject<UExponentialHeightFogComponent>	("ExponentialHeightFog");
	SkyAtmosphere			= CreateDefaultSubobject<USkyAtmosphereComponent>			("SkyAtmosphere");
	SkyLight				= CreateDefaultSubobject<USkyLightComponent>				("SkyLight");
	PostProcess				= CreateDefaultSubobject<UPostProcessComponent>				("PostProcess");

}

// Called when the game starts or when spawned
void AWeatherManager::BeginPlay()
{
	Super::BeginPlay();
	
	VolumetricCloud = Cast<AWeatherVolumetricCloud>(UGameplayStatics::GetActorOfClass(this, AWeatherVolumetricCloud::StaticClass()));
	WaterBodyOcean = Cast<AWeatherWaterBodyOcean>(UGameplayStatics::GetActorOfClass(this, AWeatherWaterBodyOcean::StaticClass()));
	
	if (IsValid(VolumetricCloud))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: VolumetricCloud is Valid."),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
	}
	if (IsValid(WaterBodyOcean))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: WaterBodyOcean is Valid"),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
	}
	PostProcess->Settings.bOverride_AutoExposureMethod = true;
	PostProcess->Settings.bOverride_AutoExposureBias = true;
	PostProcess->Settings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
	PostProcess->Settings.AutoExposureBias = 12.f;
}

// Called every frame
void AWeatherManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeatherManager::RunTestWaterBodyFunction(int TargetWaveIdx, float LerpDuration)
{
	UE_LOG(LogTemp, Error, TEXT("%s::%s: Start"),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
	if (IsValid(WaterBodyOcean))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Run1"),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
		WaterBodyOcean->WaveToTargetWave(TargetWaveIdx,LerpDuration);
	}
	
	if (IsValid(VolumetricCloud))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Run2"),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
		VolumetricCloud->StateToTargetState(TargetWaveIdx,LerpDuration);
		VolumetricCloud->CreateEffectRain(GetWorld()->GetFirstPlayerController());
	}
}


