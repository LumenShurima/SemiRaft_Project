// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/MovementComponent.h"
#include "SharkMovementComponent.generated.h"


USTRUCT(BlueprintType)
struct FSharkMovement
{
	GENERATED_BODY()
	
	
	
};
/**
 * 
 */
UCLASS(Blueprintable, Config = Game, meta = (BlueprintSpawnableComponent))
class SEMIRAFT_API USharkMovementComponent : public UMovementComponent
{
	GENERATED_BODY()
	
	
private:
	UPROPERTY()
	FVector CurrentVelocity = FVector::ZeroVector;
	
	UPROPERTY()
	FVector TargetPos = FVector::ZeroVector;
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shark|Movement")
	float MaxSpeed = 1600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shark|Movement")
	float MinCruiseSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shark|Movement")
	float ForwardAcceleration = 700.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shark|Movement")
	float MaxTurnAcceleration = 1200.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shark|Movement")
	float AcceptanceRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shark|Movement")
	float Deceleration = 800.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shark|Movement")
	float SlowDownRadius = 2000.0f;
	
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	
	void UpdateSharkVelocity(
		const FVector& CurrentLocation,
		const FVector& TargetLocation,
		float DeltaTime
		);
	FVector RotateDirectionToward(
		const FVector& CurrentDirection,
		const FVector& DesiredDirection,
		const float MaxRadians
		);
	void MoveShark(const float DeltaTime);
	void ApplyDeceleration(float DeltaTime);
	
	void SetTargetPos(FVector NewTargetPos);
};
