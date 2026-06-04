// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InventorySystem/InventorySystemStruct.h"
#include "ItemBase.generated.h"

class UExtendedBuoyancyComponent;

UCLASS(BlueprintType)
class SEMIRAFT_API AItemBase : public AActor
{
	GENERATED_BODY()
	
private:
	FTimerHandle SpawnTimerHandle;
	FTimerHandle SinkTimerHandle;
	FTimerHandle FinalDestroyTimerHandle;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBuoyancyType = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UExtendedBuoyancyComponent> BuoyancyComponent;
	

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FItem Data;
	
public:	
	// Sets default values for this actor's properties
	AItemBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	
	
};
