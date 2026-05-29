// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Shark.generated.h"

class UBoxComponent;
class USphereComponent;
class USharkMovementComponent;
class UActorComponent;

UCLASS()
class SEMIRAFT_API AShark : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AShark();
	
public:
	UPROPERTY()
	TObjectPtr<USceneComponent> TargetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UBoxComponent> SharkVolume;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UBoxComponent> AttackOverlap;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USphereComponent> WaterBodyCheck;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USharkMovementComponent> MovementComponent;
	
private:
	UPROPERTY()
	bool bIsWaterBodyOverlapping = false;
	
	
protected:
	
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION(BlueprintCallable)
	void SetTarget(USceneComponent* InTargetComponent);
	
	UFUNCTION()
	void OnAttackOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	
	UFUNCTION()
	void OnWaterOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	
	UFUNCTION()
	void OnWaterOverlapEnd(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	
	
};
