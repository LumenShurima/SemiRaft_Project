// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableBase.h"
#include "Trash.generated.h"

UCLASS()
class SEMIRAFT_API ATrash : public AInteractableBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATrash();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyVar")
	TObjectPtr<class UStaticMeshComponent> MeshComp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyVar")
	TObjectPtr<class UBoxComponent> CollisionComp;
	
	bool bIsHooked = false;
	
	void AttachToHook(USceneComponent* HookMesh);
	virtual void PressEKey_Implementation() override;
};
