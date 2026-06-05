// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableBase.h"
#include "Spear.generated.h"

class AMyCharacter;

UCLASS()
class SEMIRAFT_API ASpear : public AInteractableBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASpear();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyVar")
	TObjectPtr<class USceneComponent> RootComp;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyVar")
	TObjectPtr<class UStaticMeshComponent> MeshComp;
	
	UPROPERTY()
	TObjectPtr<class AMyCharacter> Player;
	
	UPROPERTY()
	TObjectPtr<class ARaftActor> RaftActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyVar")
	TObjectPtr<class UAnimInstance> AnimInstance;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyVar")
	TObjectPtr<class UAnimMontage> AttackMontage;
	
	virtual void LeftClickStarted_Implementation() override;
	virtual void AttachToPlayer_Implementation(AMyCharacter* player) override;
	virtual void DetachFromPlayer_Implementation(AMyCharacter* player) override;
};
