// Fill out your copyright notice in the Description page of Project Settings.


#include "Weather/WeatherVolumetricCloud.h"
#include "Materials/MaterialInstanceDynamic.h"

AWeatherVolumetricCloud::AWeatherVolumetricCloud()
{
	
}

void AWeatherVolumetricCloud::BeginPlay()
{
	Super::BeginPlay();
	
	VolumetricCloudComponentProxy = GetComponentByClass<UVolumetricCloudComponent>();
	if (VolumetricCloudComponentProxy)
	{
		CloudDynamicMaterial = UMaterialInstanceDynamic::Create(VolumetricCloudComponentProxy->GetMaterial(), this);
		
	}
}

void AWeatherVolumetricCloud::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (VolumetricCloudComponentProxy)
	{
		CloudDynamicMaterial->SetScalarParameterValue(TEXT("StormClouds"),StormClouds);
		CloudDynamicMaterial->SetScalarParameterValue(TEXT("Storm_LightningTexScale"),Storm_LightningTexScale);
		CloudDynamicMaterial->SetVectorParameterValue(TEXT("Storm_LightningAnim"),Storm_LightningAnim);
		CloudDynamicMaterial->SetVectorParameterValue(TEXT("Storm_LightningClouds"),Storm_LightningClouds);
		CloudDynamicMaterial->SetVectorParameterValue(TEXT("Storm_LightningColor"),Storm_LightningColor);
		CloudDynamicMaterial->SetVectorParameterValue(TEXT("Storm_LightningMasks"),Storm_LightningMasks);
		CloudDynamicMaterial->SetVectorParameterValue(TEXT("Storm_AlbedoColor"),Storm_AlbedoColor);
	}
	
}
