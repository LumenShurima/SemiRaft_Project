// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/VolumetricCloudComponent.h"
#include "WeatherVolumetricCloud.generated.h"

/**
 * 
 */
UCLASS()
class SEMIRAFT_API AWeatherVolumetricCloud : public AVolumetricCloud
{
	GENERATED_BODY()
	
	
private:
	AWeatherVolumetricCloud();
	
	
public:
	TObjectPtr<UVolumetricCloudComponent> VolumetricCloudComponentProxy;
	TObjectPtr<UMaterialInstanceDynamic> CloudDynamicMaterial;
	
};
