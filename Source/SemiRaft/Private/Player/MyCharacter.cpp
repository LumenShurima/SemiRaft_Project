// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MyCharacter.h"

#include "CableComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "BuildSystem/BuildComponent.h"
#include "BuildSystem/MakeEngineUI.h"
#include "BuildSystem/RaftActor.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InventorySystem/InventoryComponent.h"
#include "InventorySystem/InventorySlot.h"
#include "Kismet/GameplayStatics.h"
#include "Player/HookAimUI.h"
#include "ToolSystem/Axe.h"
#include "ToolSystem/Hammer.h"
#include "ToolSystem/Hook.h"
#include "ToolSystem/Spear.h"
#include "ToolSystem/Trash.h"


// Sets default values
AMyCharacter::AMyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SphereComp->SetupAttachment(this->GetMesh());
	InventoryComp = CreateDefaultSubobject<UInventoryComponent>("InventoryComp");
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	Cam = this->FindComponentByClass<UCameraComponent>();
	if (SphereComp)
	{
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AMyCharacter::OnMySphereBeginOverlap);
	}
	Mesh1P = Cast<USkeletalMeshComponent>(GetDefaultSubobjectByName(TEXT("FirstPersonMesh")));
	
	PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController)
	{
		EnableInput(PlayerController);
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
		PlayerController->ResetIgnoreMoveInput();
		PlayerController->ResetIgnoreLookInput();
		PlayerController->SetIgnoreMoveInput(false);
		PlayerController->SetIgnoreLookInput(false);
		
		auto* subsys = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (subsys)
		{
			subsys->RemoveMappingContext(IMC_Player);
			subsys->AddMappingContext(IMC_Player, 0);
		}
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->SetMovementMode(MOVE_Walking);
	}
	
	check(HookAimUIFactory);
	if (IsLocallyControlled() && HookAimUIFactory)
	{
		TArray<UUserWidget*> FoundHookAimWidgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
			GetWorld(),
			FoundHookAimWidgets,
			HookAimUIFactory,
			true
		);

		for (int32 i = 0; i < FoundHookAimWidgets.Num(); ++i)
		{
			UHookAimUI* FoundHookAimUI = Cast<UHookAimUI>(FoundHookAimWidgets[i]);
			if (!FoundHookAimUI)
			{
				continue;
			}

			if (FoundHookAimUI == HookAimUI)
			{
				HookAimUI->SetVisibility(ESlateVisibility::Visible);
			}
			else if (!IsValid(HookAimUI))
			{
				HookAimUI = FoundHookAimUI;
				HookAimUI->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				FoundHookAimUI->RemoveFromParent();
			}
		}

		if (!IsValid(HookAimUI))
		{
			if (PlayerController)
			{
				HookAimUI = Cast<UHookAimUI>(CreateWidget(PlayerController, HookAimUIFactory));
			}
			else
			{
				HookAimUI = Cast<UHookAimUI>(CreateWidget(GetWorld(), HookAimUIFactory));
			}
			
			if (HookAimUI)
			{
				HookAimUI->AddToViewport();
			}
		}

		if (HookAimUI)
		{
			HookAimUI->UpdateCurrentItemUI(1);
		}
	}
	
	check(MakeEngineUIFactory)
	if (MakeEngineUIFactory)
	{
		MakeEngineUI = Cast<UMakeEngineUI>(CreateWidget(GetWorld(), MakeEngineUIFactory));
		
		if (MakeEngineUI)
		{
			MakeEngineUI->InventoryComp = InventoryComp;
			MakeEngineUI->AddToViewport();
			MakeEngineUI->SetVisibility(ESlateVisibility::Collapsed);
			bMakeEngineUIVisible = false;
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
			IInteractInterface::Execute_AttachToPlayer(SpawnedHook, this);
			/*const FName HandSocketName = TEXT("HandGrip_R");
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
			}*/
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
		input->BindAction(IA_MyRkey, ETriggerEvent::Started, this, &AMyCharacter::OnPressedRKey);
		input->BindAction(IA_One, ETriggerEvent::Started, this, &AMyCharacter::OnPressedOneKey);
		input->BindAction(IA_Two, ETriggerEvent::Started, this, &AMyCharacter::OnPressedTwoKey);
		input->BindAction(IA_Three, ETriggerEvent::Started, this, &AMyCharacter::OnPressedThreeKey);
		input->BindAction(IA_Four, ETriggerEvent::Started, this, &AMyCharacter::OnPressedFourKey);
		input->BindAction(IA_Tab, ETriggerEvent::Started, this, &AMyCharacter::OnPressedTabKey);
		input->BindAction(IA_Q, ETriggerEvent::Started, this, &AMyCharacter::OnPressedQKey);
		
	}
}

void AMyCharacter::OnMySphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 훅에 있는 아이템 제거
	ATrash* trash = Cast<ATrash>(OtherActor);
	if (trash && trash->bIsHooked)
	{
		// 인벤토리 저장
		if (InventoryComp)
		{
			InventoryComp->PickUpItem(trash);
		}
		trash->Destroy();
	}
}

void AMyCharacter::UpdatedChargingUI()
{
	if (!IsValid(HookAimUI))
	{
		return;
	}

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
	// 인벤토리에 저장
	if (InventoryComp)
	{
		InventoryComp->PickUpItem(CurrentTarget);
	}
}

void AMyCharacter::OnLeftClickStarted()
{
	if (bMakeEngineUIVisible)
	{
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("좌클!!!!!"));
	if (CurrentItem == nullptr) return;
	
	IInteractInterface::Execute_LeftClickStarted(CurrentItem);
}

void AMyCharacter::OnLeftClickTriggered()
{
	if (bMakeEngineUIVisible)
	{
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("좌클 누르는 중"));
	if (CurrentItem == nullptr) return;
	IInteractInterface::Execute_LeftClickTriggered(CurrentItem);
	UpdatedChargingUI();
}

void AMyCharacter::OnLeftClickCompleted()
{
	if (bMakeEngineUIVisible)
	{
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("좌클 끝"));
	if (CurrentItem == nullptr) return;
	
	IInteractInterface::Execute_LeftClickCompleted(CurrentItem);
	UpdatedChargingUI();
}

void AMyCharacter::OnRightClickStarted()
{
	if (bMakeEngineUIVisible)
	{
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("우클 시작"));
	if (CurrentItem == nullptr) return;
	
	IInteractInterface::Execute_RightClickStarted(CurrentItem);
}

void AMyCharacter::InteractionCheck()
{
	if (!Cam)
	{
		UE_LOG(LogTemp, Error, TEXT("카메라 NULL"));
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
		//UE_LOG(LogTemp, Log, TEXT("인터렉터블 액터 감지!!"));
		if (CurrentTarget != HitResult.GetActor())
		{
			//UE_LOG(LogTemp, Log, TEXT("저장된 것과 다른 액터!!"));
			CurrentTarget = Cast<AItemBase>(HitResult.GetActor());
			if (!CurrentItem.IsA(AHammer::StaticClass()) && CurrentTarget)
			{
				// UI에 이름 띄우기 등의 로직 수행
				HookAimUI->HorizontalBoxActive(2, true);
			}
		}
	}
	else
	{
		HookAimUI->HorizontalBoxActive(2, false);
		CurrentTarget = nullptr;
	}
}

void AMyCharacter::OnPressedRKey()
{
	if (RaftActor == nullptr)
	{
		RaftActor = Cast<ARaftActor>(
		   UGameplayStatics::GetActorOfClass(GetWorld(), ARaftActor::StaticClass())
	   );
	}

	if (!RaftActor || !RaftActor->BuildComponent)
	{
		return;
	}

	RaftActor->BuildComponent->bRoofFlipped = !RaftActor->BuildComponent->bRoofFlipped;
}

void AMyCharacter::OnPressedOneKey()
{
	if (CurrentItem && CurrentItem->IsA(AHook::StaticClass()))
	{
		return;
	}
	
	if (CurrentItem && CurrentItem->GetClass()->ImplementsInterface(UInteractInterface::StaticClass()))
	{
		IInteractInterface::Execute_DetachFromPlayer(CurrentItem, this);
	}

	if (!HookClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	AHook* SpawnedHook = GetWorld()->SpawnActor<AHook>(
		HookClass,
		GetActorLocation(),
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (!SpawnedHook)
	{
		return;
	}

	CurrentItem = SpawnedHook;

	if (CurrentItem->GetClass()->ImplementsInterface(UInteractInterface::StaticClass()))
	{
		IInteractInterface::Execute_AttachToPlayer(CurrentItem, this);
	}
	
	if (HookAimUI)
	{
		HookAimUI->UpdateCurrentItemUI(1);
		HookAimUI->HorizontalBoxActive(0, false);
		HookAimUI->HorizontalBoxActive(1, false);
	}
}

void AMyCharacter::OnPressedTwoKey()
{
	if (CurrentItem && CurrentItem->IsA(AHammer::StaticClass()))
	{
		return;
	}
	
	if (CurrentItem && CurrentItem->GetClass()->ImplementsInterface(UInteractInterface::StaticClass()))
	{
		IInteractInterface::Execute_DetachFromPlayer(CurrentItem, this);
	}

	if (!HammerClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	AHammer* SpawnedHammer = GetWorld()->SpawnActor<AHammer>(
		HammerClass,
		GetActorLocation(),
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (!SpawnedHammer)
	{
		return;
	}

	CurrentItem = SpawnedHammer;

	if (CurrentItem->GetClass()->ImplementsInterface(UInteractInterface::StaticClass()))
	{
		IInteractInterface::Execute_AttachToPlayer(CurrentItem, this);
	}
	
	if (HookAimUI)
	{
		HookAimUI->UpdateCurrentItemUI(2);
		HookAimUI->HorizontalBoxActive(0, true);
	}
}

void AMyCharacter::OnPressedThreeKey()
{
	if (CurrentItem && CurrentItem->IsA(AAxe::StaticClass()))
	{
		return;
	}
	
	if (CurrentItem && CurrentItem->GetClass()->ImplementsInterface(UInteractInterface::StaticClass()))
	{
		IInteractInterface::Execute_DetachFromPlayer(CurrentItem, this);
	}

	if (!AxeClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	AAxe* SpawnedAxe = GetWorld()->SpawnActor<AAxe>(
		AxeClass,
		GetActorLocation(),
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (!SpawnedAxe)
	{
		return;
	}

	CurrentItem = SpawnedAxe;

	if (CurrentItem->GetClass()->ImplementsInterface(UInteractInterface::StaticClass()))
	{
		IInteractInterface::Execute_AttachToPlayer(CurrentItem, this);
	}
	
	if (HookAimUI)
	{
		HookAimUI->UpdateCurrentItemUI(3);
		HookAimUI->HorizontalBoxActive(0, false);
		HookAimUI->HorizontalBoxActive(1, false);
	}
}

void AMyCharacter::OnPressedFourKey()
{
	if (CurrentItem && CurrentItem->IsA(ASpear::StaticClass()))
	{
		return;
	}
	
	if (CurrentItem && CurrentItem->GetClass()->ImplementsInterface(UInteractInterface::StaticClass()))
	{
		IInteractInterface::Execute_DetachFromPlayer(CurrentItem, this);
	}

	if (!SpearClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	ASpear* SpawnedSpear = GetWorld()->SpawnActor<ASpear>(
		SpearClass,
		GetActorLocation(),
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (!SpawnedSpear)
	{
		return;
	}

	CurrentItem = SpawnedSpear;

	if (CurrentItem->GetClass()->ImplementsInterface(UInteractInterface::StaticClass()))
	{
		IInteractInterface::Execute_AttachToPlayer(CurrentItem, this);
	}
	
	if (HookAimUI)
	{
		HookAimUI->UpdateCurrentItemUI(4);
		HookAimUI->HorizontalBoxActive(0, false);
		HookAimUI->HorizontalBoxActive(1, false);
	}
}

void AMyCharacter::OnPressedTabKey()
{
	if (bMakeEngineUIVisible)
	{
		MakeEngineUI->SetVisibility(ESlateVisibility::Collapsed);
		bMakeEngineUIVisible = false;
		PlayerController->SetShowMouseCursor(false);
		PlayerController->SetInputMode(FInputModeGameOnly());
	}
	else
	{
		MakeEngineUI->UpdateItemCount();
		MakeEngineUI->SetVisibility(ESlateVisibility::Visible);
		bMakeEngineUIVisible = true;
		PlayerController->SetShowMouseCursor(true);
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(MakeEngineUI->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

		PlayerController->SetInputMode(InputMode);
	}
}

void AMyCharacter::OnPressedQKey()
{
	if (!InventoryComp)
	{
		return;
	}
	if (!PlayerController)
	{
		return;
	}

	if (!bInventoryOpen)
	{
		InventoryComp->CreateInventoryWidget(PlayerController);
		bInventoryOpen = true;
	}
	else
	{
		InventoryComp->DestroyInventoryWidget(PlayerController);
		bInventoryOpen = false;
	}
}

void AMyCharacter::RemoveHookAimUI()
{UE_LOG(LogTemp, Warning, TEXT("RemoveHookAimUI Called"));
	if (HookAimUI)
	{
		UE_LOG(LogTemp, Warning, TEXT("HookAimUI Valid"));
		HookAimUI->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HookAimUI Invalid"));
	}
}

