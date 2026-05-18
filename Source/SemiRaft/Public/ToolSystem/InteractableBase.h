// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractInterface.h"
#include "InventorySystem/ItemBase.h"
#include "InteractableBase.generated.h"

UCLASS()
class SEMIRAFT_API AInteractableBase : public AItemBase, public IInteractInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInteractableBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
