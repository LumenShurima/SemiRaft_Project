// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#define FloorClass ABuoyancyRootActor

#include "CoreMinimal.h"
#include "BuoyancyComponent.h"
#include "RaftPlatformBuoyancyComponent.generated.h"


/**
 * 
 */
UCLASS(Blueprintable, Config = Game, meta = (BlueprintSpawnableComponent))
class SEMIRAFT_API URaftPlatformBuoyancyComponent : public UBuoyancyComponent
{
	GENERATED_BODY()
	
    URaftPlatformBuoyancyComponent(const FObjectInitializer& ObjectInitializer);
    virtual void BeginPlay() override;
    
    UFUNCTION(BlueprintCallable)
    FORCEINLINE void SetBuoyancyCoefficient(float NewCoefficient) { BuoyancyData.BuoyancyCoefficient = NewCoefficient; }

    UFUNCTION(BlueprintPure)
    FORCEINLINE float GetBuoyancyCoefficient() { return BuoyancyData.BuoyancyCoefficient; }

public:
    // Calculate the pontoons radius to have neutral buoyancy at BuoyancyCoefficient = 1.f
    UFUNCTION(BlueprintCallable)
    void ComputePontoonsForRaft();

protected:
    

    // Will override pontoons radius at BeginPlay when enabled,
    // it allows to set buoyancy easily :
    // BuoyancyCoefficient >  1 => buoyant
    // BuoyancyCoefficient == 1 => static
    // BuoyancyCoefficient <  1 => sinking
    UPROPERTY(EditDefaultsOnly, category = "Buoyancy Extended")
    bool bInitWithNeutralBuoyancy = true;

	UPROPERTY(EditDefaultsOnly, category = "Buoyancy Extended")
	float GridSize = 500.f;
	UPROPERTY(EditDefaultsOnly, category = "Buoyancy Extended")
	float PontoonZ = 0.f;
		
		
};

