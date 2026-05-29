// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Shark.h"

#include "MaterialStatsCommon.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Enemy/SharkMovementComponent.h"
#include "BuildSystem/RaftActor.h"
#include "Components/StaticMeshComponent.h"


// Sets default values
AShark::AShark()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	if (!IsValid(Mesh))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Create Skeletal Mesh Component Failed. "), *GetClass()->GetName(), TEXT(__FUNCTION__));
	}
	else
	{
		ConstructorHelpers::FObjectFinder<USkeletalMesh> SharkMeshAsset (TEXT("/Game/Shark/Shark.Shark"));
		
		if (SharkMeshAsset.Succeeded())
		{
			Mesh->SetSkeletalMesh(SharkMeshAsset.Object);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%s: Shark Mesh Asset Is Cant Be Found."), *GetClass()->GetName(), TEXT(__FUNCTION__));
		}
		
	}
	
	SharkVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("SharkVolume"));
	if (!IsValid(SharkVolume))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Create SharkVolume Box Component Failed. "), *GetClass()->GetName(), TEXT(__FUNCTION__));
	}
	else
	{
		SetRootComponent(SharkVolume);
		SharkVolume->SetBoxExtent(FVector(350.f, 100.f, 80.f));
		SharkVolume->SetSimulatePhysics(true);

		/* ———————————————————————————————————Set Primitive Target————————————————————————————————————————— */
		UPrimitiveComponent* CollisionComponent = SharkVolume;
		/* ———————————————————————————————————————————————————————————————————————————————————————————————— */

		/* ——————————————————————————————————Default Engine Collision Setting—————————————————————————————— */
		// using Event Enable
		CollisionComponent->SetGenerateOverlapEvents(true);
		// Collision Setting
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);

		// Trace Channel
		CollisionComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		CollisionComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

		// Object Channel
		CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		CollisionComponent->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
		CollisionComponent->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
		/* ———————————————————————————————————————————————————————————————————————————————————————————————— */
		
		// Custom Channel
		SharkVolume->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);		// Hook
		SharkVolume->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Ignore);		// ECC_Interactable
		SharkVolume->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Block);		// Building
		
		
	}
	
	AttackOverlap = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackOverlap"));
	if (!IsValid(AttackOverlap))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Create AttackOverlap Box Component Failed. "), *GetClass()->GetName(), TEXT(__FUNCTION__));
	}
	else
	{
		AttackOverlap->SetBoxExtent(FVector(30.f, 50.f, 65.f));
		
		/* ———————————————————————————————————Set Primitive Target————————————————————————————————————————— */
		UPrimitiveComponent* CollisionComponent = AttackOverlap;
		/* ———————————————————————————————————————————————————————————————————————————————————————————————— */
		
		CollisionComponent->SetRelativeLocation(FVector(0.f,340.f,0.f));

		/* ——————————————————————————————————Default Engine Collision Setting—————————————————————————————— */
		// using Event Enable
		CollisionComponent->SetGenerateOverlapEvents(true);
		// Collision Setting
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);

		// Trace Channel
		CollisionComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		CollisionComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

		// Object Channel
		CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		CollisionComponent->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
		CollisionComponent->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
		/* ———————————————————————————————————————————————————————————————————————————————————————————————— */
		
		// Custom Channel
		AttackOverlap->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);		// Hook
		AttackOverlap->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Ignore);		// ECC_Interactable
		AttackOverlap->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Ignore);		// Building
		
		
		// Custom Channel
		AttackOverlap->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Ignore);		// ECC_Interactable
		AttackOverlap->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);		// Hook
		AttackOverlap->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);	// Building
		
		
		AttackOverlap->OnComponentBeginOverlap.AddDynamic(
			this,
			&AShark::OnAttackOverlapBegin);
	}
	
	WaterBodyCheck = CreateDefaultSubobject<USphereComponent>(TEXT("WaterBodyCheck"));
	if (!IsValid(WaterBodyCheck))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Create WaterBodyCheck Sphere Component Failed. "), *GetClass()->GetName(), TEXT(__FUNCTION__));
	}
	else
	{
		/* ———————————————————————————————————Set Primitive Target————————————————————————————————————————— */
		UPrimitiveComponent* CollisionComponent = WaterBodyCheck;
		/* ———————————————————————————————————————————————————————————————————————————————————————————————— */
		
		CollisionComponent->SetRelativeLocation(FVector(0.f, -35.f, 130.f));


		/* ——————————————————————————————————Default Engine Collision Setting—————————————————————————————— */
		// using Event Enable
		CollisionComponent->SetGenerateOverlapEvents(true);
		// Collision Setting
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);

		// Trace Channel
		CollisionComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		CollisionComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

		// Object Channel
		CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
		CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
		CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		CollisionComponent->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
		CollisionComponent->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
		/* ———————————————————————————————————————————————————————————————————————————————————————————————— */
		
		// Custom Channel
		WaterBodyCheck->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap); // ECC_Interactable
		WaterBodyCheck->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);	// Hook
		WaterBodyCheck->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Ignore);	// Building
		
		WaterBodyCheck->OnComponentBeginOverlap.AddDynamic(
			this,
			&AShark::OnWaterOverlapBegin);
		
		WaterBodyCheck->OnComponentBeginOverlap.AddDynamic(
			this,
			&AShark::OnWaterOverlapEnd);
	}
	
	MovementComponent = CreateDefaultSubobject<USharkMovementComponent>(TEXT("MovementComponent"));
	if (!IsValid(MovementComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Create Shark Movement Component Failed. "), *GetClass()->GetName(), TEXT(__FUNCTION__));
	}
	else
	{
		
	}
	
	
}

// Called when the game starts or when spawned
void AShark::BeginPlay()
{
	Super::BeginPlay();
	
	
}

// Called every frame
void AShark::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	SharkVolume->SetSimulatePhysics(!bIsWaterBodyOverlapping);
}

// Called to bind functionality to input
void AShark::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AShark::SetTarget(USceneComponent* InTargetComponent)
{
	TargetComponent = InTargetComponent;
	FVector TargetPos = TargetComponent->GetComponentLocation();
	MovementComponent->SetTargetPos(TargetPos);
}

void AShark::OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ARaftActor* RaftActor = Cast<ARaftActor>(OtherActor);
	if (!IsValid(RaftActor)) return;
	UStaticMeshComponent* RaftMesh = Cast<UStaticMeshComponent>(OverlappedComponent);
	if (!IsValid(RaftMesh)) return;
	
	for (auto It = RaftActor->GridMap.CreateIterator(); It; ++It)
	{
		const FIntVector& Key = It.Key();
		UStaticMeshComponent* StaticMeshComponent = It.Value();
		
		if (TargetComponent == RaftMesh)
		{
			AttackOverlap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			TargetComponent = nullptr;
			RaftActor->DestroyBlockAndCheckStability(Key);
		}
	}
}

void AShark::OnWaterOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	bIsWaterBodyOverlapping = true;
}

void AShark::OnWaterOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	bIsWaterBodyOverlapping = false;
}

