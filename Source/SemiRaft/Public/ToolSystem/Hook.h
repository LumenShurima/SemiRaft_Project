// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableBase.h"
#include "Hook.generated.h"

UENUM(BlueprintType)
enum class EHookState : uint8
{
	IDLE,
	CHARGING,
	LAUNCHING,
	HOOKED,
	RETURNING,
	CRASHED
};

UCLASS()
class SEMIRAFT_API AHook : public AInteractableBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AHook();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyVar")
	TObjectPtr<class UStaticMeshComponent> MeshComp;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyVar")
	TObjectPtr<class UCableComponent> CableComp;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyVar")
	TObjectPtr<class USphereComponent> CollisionComp;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyVar")
	EHookState HookState;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyVar")
	float ChargingTime = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyVar")
	float MaxChargingTime = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyVar")
	float MinLaunchingSpeed = 2000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyVar")
	float MaxLaunchingSpeed = 6000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyVar")
	float ReturnSpeed = 600.f;
	
	UPROPERTY()
	class AMyCharacter* Player;
	
	virtual void LeftClickStarted_Implementation() override;
	virtual void LeftClickTriggered_Implementation() override;
	virtual void LeftClickCompleted_Implementation() override;
	virtual void RightClickStarted_Implementation() override;
	virtual void AttachToPlayer_Implementation(AMyCharacter* player) override;
	virtual void DetachFromPlayer_Implementation(AMyCharacter* player) override;
	
	void Launch();
	void Returning();
	
	UFUNCTION(BlueprintCallable)
	void FastReturn();
};
