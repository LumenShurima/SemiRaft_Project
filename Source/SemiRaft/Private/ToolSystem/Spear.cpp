// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSystem/Spear.h"

#include "EngineUtils.h"
#include "BuildSystem/BuildComponent.h"
#include "BuildSystem/RaftActor.h"
#include "Player/MyCharacter.h"


// Sets default values
ASpear::ASpear()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootComp);
	
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetupAttachment(RootComp);
}

// Called when the game starts or when spawned
void ASpear::BeginPlay()
{
	Super::BeginPlay();
	MeshComp->SetSimulatePhysics(false);
}

// Called every frame
void ASpear::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASpear::LeftClickStarted_Implementation()
{
	Super::LeftClickStarted_Implementation();
	if (!AnimInstance)
	{
		AnimInstance = Player->GetMesh()->GetAnimInstance();
	}
	
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage);
	}
}

void ASpear::AttachToPlayer_Implementation(AMyCharacter* player)
{
	Super::AttachToPlayer_Implementation(player);
	const FName HandSocketName = TEXT("HandGrip_R");
	Player = player;
	
	TArray<UActorComponent*> MeshComponents;
	Player->GetComponents(USkeletalMeshComponent::StaticClass(), MeshComponents);

	USkeletalMeshComponent* TargetMesh = nullptr;

	for (UActorComponent* Component : MeshComponents)
	{
		if (Component && Component->GetName().Contains(TEXT("FirstPersonMesh")))
		{
			TargetMesh = Cast<USkeletalMeshComponent>(Component);
			break;
		}
	}
	if (TargetMesh->DoesSocketExist(HandSocketName))
	{
		this->AttachToComponent(TargetMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandSocketName);
		UE_LOG(LogTemp, Warning, TEXT("Spear가 소켓에 성공적으로 부착되었습니다."));
		
		if (RaftActor == nullptr)
		{
			for (TActorIterator<ARaftActor> It(GetWorld()); It; ++It)
			{
				ARaftActor* FoundRaft = *It;
				if (FoundRaft)
				{
					// 찾은 뗏목을 변수에 저장하고 루프 탈출
					RaftActor = FoundRaft;
					UE_LOG(LogTemp, Log, TEXT("월드에서 뗏목 액터를 찾아 연결했습니다."));
					break; 
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("소켓을 찾을 수 없습니다: %s"), *HandSocketName.ToString());
	}
}

void ASpear::DetachFromPlayer_Implementation(AMyCharacter* player)
{
	Super::DetachFromPlayer_Implementation(player);
	Destroy();
}

