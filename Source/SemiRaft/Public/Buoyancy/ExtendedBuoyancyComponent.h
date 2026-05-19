// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuoyancyComponent.h"
#include "ExtendedBuoyancyComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, Config = Game, meta = (BlueprintSpawnableComponent))
class SEMIRAFT_API UExtendedBuoyancyComponent : public UBuoyancyComponent
{
	GENERATED_BODY()
	UExtendedBuoyancyComponent(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetBuoyancyCoefficient(float NewCoefficient) { BuoyancyData.BuoyancyCoefficient = NewCoefficient; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetBuoyancyCoefficient() { return BuoyancyData.BuoyancyCoefficient; }

public:
	// Calculate the pontoons radius to have neutral buoyancy at BuoyancyCoefficient = 1.f
	UFUNCTION(BlueprintCallable)
	void ComputePontoonsRadiusForNeutralBuoyancy();
protected:
	

	// Will override pontoons radius at BeginPlay when enabled,
	// it allows to set buoyancy easily :
	// BuoyancyCoefficient >  1 => buoyant
	// BuoyancyCoefficient == 1 => static
	// BuoyancyCoefficient <  1 => sinking
	UPROPERTY(EditDefaultsOnly, category = "Buoyancy Extended")
	bool bInitWithNeutralBuoyancy = true;
};
