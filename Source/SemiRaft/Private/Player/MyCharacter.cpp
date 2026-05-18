// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MyCharacter.h"

#include "CableComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Player/HookAimUI.h"
#include "ToolSystem/Hook.h"
#include "ToolSystem/Trash.h"


// Sets default values
AMyCharacter::AMyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SphereComp->SetupAttachment(this->GetMesh());
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (SphereComp)
	{
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AMyCharacter::OnMySphereBeginOverlap);
	}
	Mesh1P = Cast<USkeletalMeshComponent>(GetDefaultSubobjectByName(TEXT("FirstPersonMesh")));
	
	auto* pc = Cast<APlayerController>(Controller);
	if (pc)
	{
		auto* subsys = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pc->GetLocalPlayer());
		if (subsys)
		{
			subsys->RemoveMappingContext(IMC_Player);
			subsys->AddMappingContext(IMC_Player, 0);
		}
	}
	
	check(HookAimUIFactory);
	if (HookAimUIFactory)
	{
		HookAimUI = Cast<UHookAimUI>(CreateWidget(GetWorld(), HookAimUIFactory));
		
		if (HookAimUI)
		{
			HookAimUI->AddToViewport();
		}
	}
	
	if (HookClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		
		AHook* SpawnedHook = GetWorld()->SpawnActor<AHook>(HookClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		SpawnedHook->MeshComp->SetSimulatePhysics(false);
		
		if (SpawnedHook)
		{
			CurrentItem = SpawnedHook;
			const FName HandSocketName = TEXT("HandGrip_R");
			SpawnedHook->player = this;
			
			if (GetMesh()->DoesSocketExist(HandSocketName))
			{
				SpawnedHook->AttachToComponent(Mesh1P, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HandSocketName);
				UCableComponent* cable = SpawnedHook->CableComp;
				if (cable)
				{
					cable->SetAttachEndTo(this, "FirstPersonMesh", TEXT("HandGrip_L"));
				
				}
				UE_LOG(LogTemp, Warning, TEXT("Hook이 소켓에 성공적으로 부착되었습니다."));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("소켓을 찾을 수 없습니다: %s"), *HandSocketName.ToString());
			}
		}
	}
	
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	InteractionCheck();
}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	auto* input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (input)
	{
		input->BindAction(IA_MyInteract, ETriggerEvent::Started, this, &AMyCharacter::OnPressedEKey);
		input->BindAction(IA_LeftClick, ETriggerEvent::Started, this, &AMyCharacter::OnLeftClickStarted);
		input->BindAction(IA_LeftClick, ETriggerEvent::Triggered, this, &AMyCharacter::OnLeftClickTriggered);
		input->BindAction(IA_LeftClick, ETriggerEvent::Completed, this, &AMyCharacter::OnLeftClickCompleted);
		input->BindAction(IA_RightClick, ETriggerEvent::Started, this, &AMyCharacter::OnRightClickStarted);
		
	}
}

void AMyCharacter::OnMySphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 훅에 있는 아이템 제거
	ATrash* trash = Cast<ATrash>(OtherActor);
	if (trash && trash->bIsHooked)
	{
		trash->Destroy();
	}
}

void AMyCharacter::UpdatedChargingUI()
{
	if (CurrentItem->IsA(AHook::StaticClass()))
	{
		AHook* MyHook = Cast<AHook>(CurrentItem);
		float Percent = MyHook->ChargingTime / MyHook->MaxChargingTime;
		HookAimUI->UpdatePercent(Percent);
	}
}

void AMyCharacter::OnPressedEKey()
{
	if (CurrentTarget == nullptr) return;
	// 저장한 CurrentTargetItem의 PressEKey함수 실행
	IInteractInterface::Execute_PressEKey(CurrentTarget);
}

void AMyCharacter::OnLeftClickStarted()
{
	UE_LOG(LogTemp, Log, TEXT("좌클!!!!!"));
	if (CurrentItem == nullptr) return;
	
	IInteractInterface::Execute_LeftClickStarted(CurrentItem);
}

void AMyCharacter::OnLeftClickTriggered()
{
	UE_LOG(LogTemp, Log, TEXT("좌클 누르는 중"));
	if (CurrentItem == nullptr) return;
	IInteractInterface::Execute_LeftClickTriggered(CurrentItem);
	UpdatedChargingUI();
}

void AMyCharacter::OnLeftClickCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("좌클 끝"));
	if (CurrentItem == nullptr) return;
	
	IInteractInterface::Execute_LeftClickCompleted(CurrentItem);
	UpdatedChargingUI();
}

void AMyCharacter::OnRightClickStarted()
{
	UE_LOG(LogTemp, Log, TEXT("우클 시작"));
	if (CurrentItem == nullptr) return;
	
	IInteractInterface::Execute_RightClickStarted(CurrentItem);
}

void AMyCharacter::InteractionCheck()
{
	UCameraComponent* Cam = this->FindComponentByClass<UCameraComponent>();
	if (!Cam)
	{
		UE_LOG(LogTemp, Error, TEXT("--- [위험] 카메라 컴포넌트가 NULL입니다! ---"));
		return;
	};

	FVector Start = Cam->GetComponentLocation();
	FVector End = Start + (Cam->GetForwardVector() * 250.0f); 

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult, 
		Start, 
		End, 
		ECollisionChannel::ECC_GameTraceChannel2,
		Params
	);
	
	// 라인 디버그
	/*DrawDebugLine(
		GetWorld(),
		Start,
		End,
		bHit ? FColor::Green : FColor::Red,
		false,      // 지속 여부
		0.1f,       // 유지 시간
		0,
		0.2f        // 두께
	);*/

	if (bHit && HitResult.GetActor())
	{
		UE_LOG(LogTemp, Log, TEXT("인터렉터블 액터 감지!!"));
		if (CurrentTarget != HitResult.GetActor())
		{
			UE_LOG(LogTemp, Log, TEXT("저장된 것과 다른 액터!!"));
			CurrentTarget = Cast<AItemBase>(HitResult.GetActor());
			// UI에 이름 띄우기 등의 로직 수행
		}
	}
	else
	{
		CurrentTarget = nullptr;
	}
}

