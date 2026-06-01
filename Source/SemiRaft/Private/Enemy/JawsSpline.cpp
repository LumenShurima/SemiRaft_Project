// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/JawsSpline.h"

#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"

// Sets default values
AJawsSpline::AJawsSpline()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	auto SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	
	SetRootComponent(SceneComponent);
	
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	if (!IsValid(SplineComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Create Spline Component is Failed."), *GetClass()->GetName(), TEXT(__FUNCTION__));
	}
	else
	{
		SplineComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	}
	
	TargetSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("TargetLocation"));
	if (!IsValid(TargetSphereComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Create Target Sphere Component is Failed."), *GetClass()->GetName(), TEXT(__FUNCTION__));
	}
	else
	{
		/* ———————————————————————————————————Set Primitive Target————————————————————————————————————————— */
		UPrimitiveComponent* CollisionComponent = TargetSphereComponent;
		/* ———————————————————————————————————————————————————————————————————————————————————————————————— */
		

		/* ——————————————————————————————————Default Engine Collision Setting—————————————————————————————— */
		// using Event Enable
		CollisionComponent->SetGenerateOverlapEvents(false);
		// Collision Setting
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);

		// Trace Channel
		CollisionComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		CollisionComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

		// Object Channel
		CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
		CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
		CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		CollisionComponent->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
		CollisionComponent->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
		/* ———————————————————————————————————————————————————————————————————————————————————————————————— */
		TargetSphereComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
	}

}

// Called when the game starts or when spawned
void AJawsSpline::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AJawsSpline::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	
	if (IsValid(SplineComponent) && IsValid(TargetSphereComponent))
	{
		CurrentSplineDistance += (DeltaTime * Speed);
		float Length = SplineComponent->GetSplineLength();
		if (CurrentSplineDistance > Length) CurrentSplineDistance = 0.f;
	
		FVector WorldPos = SplineComponent->GetLocationAtDistanceAlongSpline(CurrentSplineDistance, ESplineCoordinateSpace::World);
		TargetSphereComponent->SetWorldLocation(WorldPos);
	}
}

