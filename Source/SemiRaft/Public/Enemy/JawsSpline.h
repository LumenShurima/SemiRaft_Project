// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JawsSpline.generated.h"

UCLASS()
class SEMIRAFT_API AJawsSpline : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJawsSpline();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<class USplineComponent> SplineComponent;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<class USphereComponent> TargetSphereComponent;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float CurrentSplineDistance;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Speed = 2000.f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
