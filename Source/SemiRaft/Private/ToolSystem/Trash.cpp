// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSystem/Trash.h"

#include "Components/BoxComponent.h"


// Sets default values
ATrash::ATrash()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComp"));
	SetRootComponent(CollisionComp);
	MeshComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ATrash::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATrash::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATrash::AttachToHook(USceneComponent* HookMesh)
{
	// 부유물에 시뮬레이션 있으면 다 끄고 붙여야함.
	this->AttachToComponent(HookMesh, FAttachmentTransformRules(
	EAttachmentRule::SnapToTarget, 
	EAttachmentRule::KeepWorld, 
	EAttachmentRule::KeepWorld, 
	false
	));
}

void ATrash::PressEKey_Implementation()
{
	Super::PressEKey_Implementation();
	if (false == bIsHooked)
	{
		Destroy();
	}
}

