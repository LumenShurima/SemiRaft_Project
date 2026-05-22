// Fill out your copyright notice in the Description page of Project Settings.


#include "Weather/WeatherVolumetricCloud.h"

AWeatherVolumetricCloud::AWeatherVolumetricCloud()
{
	VolumetricCloudComponentProxy = GetComponentByClass<UVolumetricCloudComponent>();
	if (VolumetricCloudComponentProxy)
	{
		CloudDynamicMaterial = UMaterialInstanceDynamic::Create(VolumetricCloudComponentProxy->GetMaterial(), this);
		CloudDynamicMaterial->SetScalarParameterValue(TEXT(""), 13);
	}
}
