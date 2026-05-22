// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "EncounterSubSystem.generated.h"


class UWaterSubsystem;
class AItemBase;
class UVolumetricCloudComponent;
class UMaterialInstanceDynamic;
DECLARE_STATS_GROUP(TEXT("MySubsystem"), STATGROUP_EncounterSubSystem, STATCAT_Advanced);

/**
 * 
 */
UCLASS(Blueprintable)
class SEMIRAFT_API UEncounterSubSystem 
	: public UGameInstanceSubsystem
	, public FTickableGameObject
{
	GENERATED_BODY()
	
private:
	FDelegateHandle WorldInitHandle;
	
	UPROPERTY()
	TObjectPtr<UWaterSubsystem> WaterSubsystem;
	
	UPROPERTY()
	TObjectPtr<UVolumetricCloudComponent> VolumetricCloudComponent;
	
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UEncounterSubSystem, STATGROUP_EncounterSubSystem);
	}
	
	virtual bool IsTickable() const override
	{
		return !IsTemplate()
			&& !IsUnreachable()
			&& GetGameInstance() != nullptr;
	}
	
	void PostWorldInit(UWorld* World, const UWorld::InitializationValues IVS);
	void PostBeginPlay();

	UFUNCTION(BlueprintCallable, Category="EncounterSubsystem", meta=(WorldContext="WorldContextObject", ReturnDisplayName="Encounter Subsystem"))
	UVolumetricCloudComponent* GetVolumetricCloudComponent();
	
	UFUNCTION(BlueprintCallable, Category = "EncounterSubsystem")
	static UEncounterSubSystem* Get(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, Category = "EncounterSubsystem", meta=(ReturnDisplayName="Item Base Ref"))
	AItemBase* SpawnScrap(FTransform SpawnTransform, TSubclassOf<AItemBase> ItemClass);
};
