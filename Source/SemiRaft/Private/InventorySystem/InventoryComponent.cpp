// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/InventoryComponent.h"
#include "InventorySystem/InventorySystemWidget.h"
#include "InventorySystem/InventorySubsystem.h"
#include "InventorySystem/ItemBase.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	ItemArray.SetNum(InventorySize);
	
	for (int i = 0; i < InventorySize; i++)
	{
		ItemArray[i].Index = i; 
	}
	
	// ...
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// ...
	
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UInventoryComponent::CreateInventoryWidget(APlayerController* PlayerController)
{
	if (!InventorySystemWidget)
	{
		if (!IsValid(PlayerController))
		{
			UE_LOG(LogTemp, Error, TEXT("UInventoryComponent: PlayerController is invalid"));
			return;
		}

		if (!InventorySystemWidgetClass)
		{
			UE_LOG(LogTemp, Error, TEXT("UInventoryComponent: InventorySystemWidgetClass is null"));
			return;
		}

		if (IsValid(InventorySystemWidget))
		{
			UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent: InventorySystemWidget already exists"));
			return;
		}

		InventorySystemWidget = CreateWidget<UInventorySystemWidget>(
			PlayerController,
			InventorySystemWidgetClass
		);

		if (!IsValid(InventorySystemWidget))
		{
			UE_LOG(LogTemp, Error, TEXT("UInventoryComponent: Failed to create InventorySystemWidget"));
			return;
		}

		InventorySystemWidget->Init(this, PlayerController);
		InventorySystemWidget->AddToViewport();

		PlayerController->SetShowMouseCursor(true);

		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(InventorySystemWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	
		PlayerController->SetInputMode(InputMode);
	}
	else
	{
		InventorySystemWidget->SetVisibility(ESlateVisibility::Visible);
	}

	UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent: InventorySystemWidget Created"));
}


void UInventoryComponent::DestroyInventoryWidget(APlayerController* PlayerController)
{
	if (!IsValid(InventorySystemWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent: InventorySystemWidget is already null"));
		return;
	}
	
	InventorySystemWidget->SetVisibility(ESlateVisibility::Hidden);

	if (IsValid(PlayerController))
	{
		PlayerController->SetShowMouseCursor(false);
		PlayerController->SetInputMode(FInputModeGameOnly());
	}

	UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent: InventorySystemWidget Removed"));
}

void UInventoryComponent::PickUpItem(AItemBase* TargetWorldItem)
{
	if (!TargetWorldItem)
	{
		return;
	}

	UInventorySubsystem* InventorySubsystem = UInventorySubsystem::Get(this);
	if (!InventorySubsystem)
	{
		return;
	}

	FItemTableRow ItemDataTableRow;
	FItem& WorldItem = TargetWorldItem->Data;

	if (!InventorySubsystem->GetItemData(WorldItem.ItemID, ItemDataTableRow))
	{
		return;
	}

	const int32 MaxStack = ItemDataTableRow.MaxStack;
	if (MaxStack <= 0)
	{
		return;
	}

	int32 RemainingStack = WorldItem.Stack;

	// 1. 같은 아이템 슬롯부터 채운다.
	for (int32 i = 0; i < ItemArray.Num(); ++i)
	{
		FItem& SlotItem = ItemArray[i];

		if (SlotItem.ItemID != WorldItem.ItemID)
		{
			continue;
		}

		if (SlotItem.Stack >= MaxStack)
		{
			continue;
		}

		const int32 Space = MaxStack - SlotItem.Stack;
		const int32 AddStack = FMath::Min(Space, RemainingStack);

		SlotItem.Stack += AddStack;
		RemainingStack -= AddStack;

		if (RemainingStack <= 0)
		{
			InventorySubsystem->DeleteWorldItem(TargetWorldItem);
			UpdateInventoryWidget();
			return;
		}
	}

	// 2. 남은 수량을 빈 슬롯에 넣는다.
	for (int32 i = 0; i < ItemArray.Num(); ++i)
	{
		FItem& SlotItem = ItemArray[i];

		if (SlotItem.ItemID != NAME_None)
		{
			continue;
		}

		const int32 AddStack = FMath::Min(MaxStack, RemainingStack);

		SlotItem = WorldItem;
		SlotItem.Stack = AddStack;
		SlotItem.Index = i;

		RemainingStack -= AddStack;

		if (RemainingStack <= 0)
		{
			InventorySubsystem->DeleteWorldItem(TargetWorldItem);
			UpdateInventoryWidget();
			return;
		}
	}

	// 3. 인벤토리가 꽉 차서 일부만 먹은 경우
	WorldItem.Stack = RemainingStack;
	UpdateInventoryWidget();
}

void UInventoryComponent::DropItem(int32 SlotIndex, int32 DropStack)
{
	UInventorySubsystem* InventorySubsystem = UInventorySubsystem::Get(this);
	
	FItemTableRow ItemDataTableRow;
	
	if (!ItemArray.IsValidIndex(SlotIndex))
	{
		return;
	}
	
	if (!InventorySubsystem->GetItemData(ItemArray[SlotIndex].ItemID, ItemDataTableRow))
	{
		return;
	}

	if (DropStack <= 0)
	{
		return;
	}

	TSubclassOf<AItemBase> WorldItemClass = ItemDataTableRow.ItemClass;
	if (!WorldItemClass)
	{
		UE_LOG(LogTemp, Error, TEXT("DropItem: WorldItemClass is null."));
		return;
	}

	FItem& SlotItem = ItemArray[SlotIndex];

	if (SlotItem.ItemID == NAME_None || SlotItem.Stack <= 0)
	{
		return;
	}

	const int32 ActualDropStack = FMath::Min(DropStack, SlotItem.Stack);

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector SpawnLocation = OwnerActor->GetActorLocation();
	FRotator SpawnRotation = OwnerActor->GetActorRotation();

	// 캐릭터 앞쪽으로 약간 떨어뜨리기
	const FVector Forward = OwnerActor->GetActorForwardVector();
	SpawnLocation += Forward * 150.0f;
	SpawnLocation.Z += 50.0f;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerActor;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AItemBase* DroppedItem = World->SpawnActor<AItemBase>(
		WorldItemClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (!DroppedItem)
	{
		return;
	}

	// 슬롯 데이터 복사
	DroppedItem->Data = SlotItem;
	DroppedItem->Data.Stack = ActualDropStack;

	// 슬롯에서 수량 차감
	SlotItem.Stack -= ActualDropStack;

	if (SlotItem.Stack <= 0)
	{
		SlotItem.ItemID = NAME_None;
		SlotItem.Stack = 0;
		SlotItem.Index = SlotIndex;

		// 필요하면 나머지 필드도 초기화
		// SlotItem.ItemName = FText::GetEmpty();
		// SlotItem.Icon = nullptr;
	}
	
	UpdateInventoryWidget();
}

void UInventoryComponent::DragDropProcess(int FromIndex, int ToIndex)
{
	if (FromIndex == ToIndex)
	{
		return;
	}
	FItem* From = &ItemArray[FromIndex];
	FItem* To = &ItemArray[ToIndex];
	UInventorySubsystem* InventorySubsystem = UInventorySubsystem::Get(this);
	if (!InventorySubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("UInventoryComponent: (DragDropProcess) InventorySubsystem is null"));
		return;
	}
	
	if (From->ItemID == To->ItemID)
	{
		FItemTableRow ItemData;
		if (!InventorySubsystem->GetItemData(To->ItemID, ItemData))
		{
			UE_LOG(LogTemp, Error, TEXT("UInventoryComponent: (DragDropProcess) InventorySubsystem Get Item Data Failed"));
			return;
		}
		
		int MaxStack = ItemData.MaxStack;
		
		int SumStack = To->Stack + From->Stack;
		if (SumStack > MaxStack)
		{
			To->Stack = MaxStack;
			From->Stack = SumStack - MaxStack;
		}
		else
		{
			To->Stack = SumStack;
			From->Stack = 0;
			From->ItemID = NAME_None;
		}
	}
	else
	{
		ItemArray.Swap(FromIndex, ToIndex);
		ItemArray[FromIndex].Index = FromIndex;
		ItemArray[ToIndex].Index = ToIndex;
	}
	
}

void UInventoryComponent::UpdateInventoryWidget()
{
	InventorySystemWidget->Update();
}

