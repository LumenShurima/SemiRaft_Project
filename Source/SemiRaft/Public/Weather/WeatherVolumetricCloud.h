// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/VolumetricCloudComponent.h"
#include "WeatherVolumetricCloud.generated.h"


class ANiagaraActor;
class UNiagaraSystem;
class UParticleSystem;

USTRUCT(BlueprintType)
struct FCloudSnapshot
{
	GENERATED_BODY()
	
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
	
	void Set(	float InStormClouds,
				float InStorm_LightningTexScale,
				FLinearColor InStorm_LightningAnim,
				FLinearColor InStorm_LightningClouds,
				FLinearColor InStorm_LightningColor,
				FLinearColor InStorm_LightningMasks,
				FLinearColor InStorm_AlbedoColor)
	{
		StormClouds = InStormClouds;
		Storm_LightningTexScale = InStorm_LightningTexScale;
		Storm_LightningAnim = InStorm_LightningAnim;
		Storm_LightningClouds = InStorm_LightningClouds;
		Storm_LightningColor = InStorm_LightningColor;
		Storm_LightningMasks = InStorm_LightningMasks;
		Storm_AlbedoColor = InStorm_AlbedoColor;
	}
};
/**
 * 
 */
UCLASS()
class SEMIRAFT_API AWeatherVolumetricCloud : public AVolumetricCloud
{
	GENERATED_BODY()
	
	
private:
	UPROPERTY()
	FCloudSnapshot CurrentSnapshot;
	
	UPROPERTY()
	FCloudSnapshot TargetSnapshot;
	
	UPROPERTY()
	bool bStorm = false;
	
	UPROPERTY()
	FTimerHandle SpawnTimerHandle;
	
	UPROPERTY()
	bool bStateToTargetState = false;
	
	UPROPERTY()
	float LerpDuration = 1.0f;
	
	UPROPERTY()
	float ElapsedDuration = 0.f;
	
	

	
	UPROPERTY()
	TObjectPtr<ANiagaraActor> RainNiagaraActor;


public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Storm")
	TObjectPtr<UNiagaraSystem> RainEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Storm")
	TObjectPtr<UParticleSystem> LightningEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Storm")
	TObjectPtr<USoundBase> LightningSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Storm")
	TArray<FCloudSnapshot> CloudPreset;
	
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
	
	AWeatherVolumetricCloud();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION()
	void StateToTargetState(int TargetIdx, float InLerpTime);
	
	UFUNCTION()
	void CreateEffectRain(APlayerController* PlayerController);
	
	UFUNCTION()
	void DeleteEffectRain();
	
};
