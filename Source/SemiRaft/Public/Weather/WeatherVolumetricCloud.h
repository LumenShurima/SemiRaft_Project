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
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Storm", meta=(ClampMin="0.0", ClampMax="1.0"))
	float StormClouds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Storm", meta=(ClampMin="0.03125", ClampMax="32.0"))
	float Storm_LightningTexScale = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Storm")
	FLinearColor Storm_LightningAnim = FLinearColor(0.0f, 0.1f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Storm")
	FLinearColor Storm_LightningClouds = FLinearColor(6.0f, 0.05f, 0.175f, 2.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Storm")
	FLinearColor Storm_LightningColor = FLinearColor(2.0f, 1.8f, 2.0f, 10.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Storm")
	FLinearColor Storm_LightningMasks = FLinearColor(10.0f, 5.0f, 0.5f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Storm")
	FLinearColor Storm_AlbedoColor = FLinearColor(0.3f, 0.3375f, 0.375f, 0.333333f);
	
public:
	TObjectPtr<UVolumetricCloudComponent> VolumetricCloudComponentProxy;
	TObjectPtr<UMaterialInstanceDynamic> CloudDynamicMaterial;
	
};
