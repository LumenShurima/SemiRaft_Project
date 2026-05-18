// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSystem/Hook.h"

#include "CableComponent.h"
#include "WaterBodyOceanActor.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "ToolSystem/Trash.h"


// Sets default values
AHook::AHook()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	CableComp = CreateDefaultSubobject<UCableComponent>("CableComp");
	CollisionComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	
	SetRootComponent(MeshComp);
	CableComp->SetupAttachment(RootComponent);
	CollisionComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AHook::BeginPlay()
{
	Super::BeginPlay();
	MeshComp->SetSimulatePhysics(false);
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
}

// Called every frame
void AHook::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHook::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	UClass* OtherClass = OtherActor->GetClass();
	// 훅이 날아가는 중일 때
	if (HookState == EHookState::LAUNCHING)
	{
		if (OtherClass == AWaterBodyOcean::StaticClass()) // 바다면 Hooked됨
		{
			FVector CharacterLocation = player->GetActorLocation();
			FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(CharacterLocation,GetActorLocation() );
			LookAtRot.Yaw += 90.f;
			float SphereRadius = CollisionComp->GetScaledSphereRadius();
			this->SetActorLocationAndRotation(GetActorLocation()+FVector(0.f, 0.f,-SphereRadius ), LookAtRot);
			//this->SetActorRotation(LookAtRot);
			MeshComp->SetSimulatePhysics(false);
			HookState = EHookState::HOOKED;
			
		}else if (OtherClass != ACharacter::StaticClass()) // 바다와 사람이 아니면 감속되고 떨어지게
		{
			//UE_LOG(LogTemp, Log, TEXT("Overlap Class : %s"), OtherActor->GetClass()->GetName());
			MeshComp->SetSimulatePhysics(true);
			MeshComp->SetLinearDamping(3.0);
			MeshComp->SetAngularDamping(3.0);
			HookState = EHookState::CRASHED;
		}
	}else if (HookState == EHookState::HOOKED || HookState == EHookState::RETURNING)
	{
		ATrash* Trash = Cast<ATrash>(OtherActor);
		if(Trash)
		{
			Trash->AttachToHook(MeshComp);
			Trash->bIsHooked = true;
		}
	}
}

void AHook::LeftClickStarted_Implementation()
{
	Super::LeftClickStarted_Implementation();
	if (HookState == EHookState::IDLE)
	{
		HookState = EHookState::CHARGING;
	}else if (HookState == EHookState::HOOKED)
	{
		HookState = EHookState::RETURNING;
	}
}

void AHook::LeftClickTriggered_Implementation()
{
	Super::LeftClickTriggered_Implementation();
	if (HookState == EHookState::CHARGING)
	{
		if (ChargingTime < MaxChargingTime)
		{
			ChargingTime += GetWorld()->GetDeltaSeconds();
		}
		
		//UI Set Percent 호출하기
		float Percent = ChargingTime / MaxChargingTime;
	}else if (HookState == EHookState::RETURNING)
	{
		Returning();
	}
}

void AHook::LeftClickCompleted_Implementation()
{
	Super::LeftClickCompleted_Implementation();
	if (HookState == EHookState::CHARGING)
	{
		Launch();
		// UI Set Percent :  0으로 바꾸기
		ChargingTime = 0.f;
	}else if (HookState == EHookState::RETURNING)
	{
		HookState = EHookState::HOOKED;
	}
}

void AHook::RightClickStarted_Implementation()
{
	Super::RightClickStarted_Implementation();
	FString StateString = UEnum::GetValueAsString(HookState);
	UE_LOG(LogTemp, Error, TEXT("현재 훅 상태: %s"), *StateString);
	if (HookState != EHookState::IDLE && HookState != EHookState::LAUNCHING)
	{	
		FastReturn();
	}
}

void AHook::Launch()
{
	if (!player) return;
	
	UCameraComponent* Cam = player->FindComponentByClass<UCameraComponent>();
	
	HookState = EHookState::LAUNCHING;
	this->DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld,EDetachmentRule::KeepWorld,true));
	MeshComp->SetSimulatePhysics(true);
	FRotator CameraRotation = Cam->GetComponentRotation();
	FRotator Rot = FRotator(CameraRotation.Pitch + 30, CameraRotation.Yaw, 0);
	float ChargingPercent = ChargingTime / MaxChargingTime;
	float LaunchSpeed = FMath::Lerp(MinLaunchingSpeed, MaxLaunchingSpeed, ChargingPercent);
	MeshComp->AddImpulse(Rot.Vector()*LaunchSpeed);
	MeshComp->AddAngularImpulseInDegrees(FVector(0.f,FMath::FRandRange(-90.f, 0.f),0.f),NAME_None ,true);
	
	ChargingTime = 0.f;
}

void AHook::Returning()
{
	auto playerVector = player->GetActorLocation();
	auto hookVector = GetActorLocation();
	FVector Dist = FVector(playerVector.X, playerVector.Y, 0.f) - hookVector;
	
	if (Dist.Length() > 30.0f)
	{
		this->SetActorLocation((hookVector + Dist.GetSafeNormal() * 600.f * GetWorld()->GetDeltaSeconds()));
	}
	else
	{
		FastReturn();
	}
}

void AHook::FastReturn()
{
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetLinearDamping(0.01f);
	MeshComp->SetAngularDamping(0.f);
	this->SetActorLocation(player->GetActorLocation(),false, nullptr ,ETeleportType::TeleportPhysics);
	this->AttachToComponent(player->GetMesh(),FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false) ,FName("HandGrip_R"));
	HookState = EHookState::IDLE;
}

