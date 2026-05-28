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
		if (!ItemArray.IsValidIndex(i)) continue;
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
	}
	else
	{
		InventorySystemWidget->SetVisibility(ESlateVisibility::Visible);
		
	}
	PlayerController->SetShowMouseCursor(true);

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(InventorySystemWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	
	PlayerController->SetInputMode(InputMode);

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
		if (!ItemArray.IsValidIndex(i)) continue;
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

	if (!IsValid(OwnerActor)) return;
	
	FVector SpawnLocation = OwnerActor->GetActorLocation();
	FRotator SpawnRotation = OwnerActor->GetActorRotation();
	
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
	}
	
	UpdateInventoryWidget();
}

FInventoryFindResult UInventoryComponent::FindItemStacks(const FName InItemID)
{
	FInventoryFindResult FindResult;
	
	if (InItemID.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: %s"),
		*GetClass()->GetName() , TEXT(__FUNCTION__)
		,TEXT("InItemID is None"));
		return FindResult;
	}
	
	for (int i = 0; i < ItemArray.Num(); i++)
	{
		if (!ItemArray.IsValidIndex(i)) continue;
		
		const FItem& Item = ItemArray[i];
		
		if (Item.ItemID != InItemID) continue;
		if (Item.Stack <= 0) continue;
		
		FindResult.FoundCount += Item.Stack;
		FindResult.ItemsIndex.Add(i);
	}
	FindResult.bSuccess = FindResult.FoundCount > 0;
	return FindResult;
}

bool UInventoryComponent::ItemConsumption(const FInventoryFindResult& InResult, int32 NumberToBeConsumed)
{
	if (NumberToBeConsumed <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Invalid consume count."),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
		return false;
	}

	if (!InResult.bSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Used the failed search results."),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
		return false;
	}

	if (InResult.ItemsIndex.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Find Result is Empty."),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
		return false;
	}

	if (InResult.FoundCount < NumberToBeConsumed)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Item Total Stack is Not Enough."),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
		return false;
	}

	int32 RemainingConsume = NumberToBeConsumed;

	for (int32 i = 0; i < InResult.ItemsIndex.Num(); ++i)
	{
		const int32 Index = InResult.ItemsIndex[i];

		if (!ItemArray.IsValidIndex(Index))
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%s: Invalid item index: %d"),
				*GetClass()->GetName(), TEXT(__FUNCTION__), Index);
			return false;
		}

		if (RemainingConsume <= 0)
		{
			break;
		}

		FItem& Item = ItemArray[Index];

		if (Item.Stack <= 0)
		{
			continue;
		}

		if (Item.Stack <= RemainingConsume)
		{
			RemainingConsume -= Item.Stack;

			Item.ItemID = NAME_None;
			Item.Stack = 0;
		}
		else
		{
			Item.Stack -= RemainingConsume;
			RemainingConsume = 0;
			break;
		}
	}
	UpdateInventoryWidget();
	return RemainingConsume <= 0;
}

void UInventoryComponent::DragDropProcess(int FromIndex, int ToIndex)
{
	if (FromIndex == ToIndex)
	{
		return;
	}
	
	if (!ItemArray.IsValidIndex(FromIndex) || !ItemArray.IsValidIndex(ToIndex)) return;
	
	UInventorySubsystem* InventorySubsystem = UInventorySubsystem::Get(this);
	if (!InventorySubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("UInventoryComponent: (DragDropProcess) InventorySubsystem is null"));
		return;
	}
	
	FItem* From = &ItemArray[FromIndex];
	FItem* To = &ItemArray[ToIndex];
	
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
	if (!IsValid(InventorySystemWidget))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Invalid InventorySystemWidget."),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
		return;
	}
	
	InventorySystemWidget->Update();
}

