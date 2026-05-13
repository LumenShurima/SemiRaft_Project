// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/InventorySubsystem.h"


void UInventorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	ItemDataTable = LoadObject<UDataTable>(
		nullptr,
		TEXT("/Game/Data/DT_ItemData.DT_ItemData")
	);
	
	BuildItemDataCache();
	
	UE_LOG(LogTemp, Log, TEXT("UInventorySubsystem: Inventory Subsystem Initialized"));
}

void UInventorySubsystem::Deinitialize()
{
	ItemDataCache.Empty();
	
	UE_LOG(LogTemp, Log, TEXT("UInventorySubsystem: Inventory Subsystem Deinitialized"));
	
	Super::Deinitialize();
}

UInventorySubsystem* UInventorySubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("UInventorySubsystem: Invalid WorldContextObject"));
		return nullptr;
	}
	
	const UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("UInventorySubsystem: Invalid World"));
		return nullptr;
	}
	
	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("UInventorySubsystem: Invalid GameInstance."));
		return nullptr;
	}
	
	UE_LOG(LogTemp, Log, TEXT("UInventorySubsystem: Get Game Instance"));
	return GameInstance->GetSubsystem<UInventorySubsystem>();
}

bool UInventorySubsystem::GetItemData(FName ItemID, FItemTableRow& OutItemData) const
{
	if (ItemID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("UInventorySubsystem: (GetItemData) ItemID is None."));
		return false;
	}
	
	if (const FItemTableRow* CacheData = ItemDataCache.Find(ItemID))
	{
		OutItemData = *CacheData;
		UE_LOG(LogTemp, Warning, TEXT("UInventorySubsystem: %s(ItemID) is already cached."), *ItemID.ToString());
		return true;
	}
	
	if (!ItemDataTable)
	{
		return false;
	}
	
	const FString ContextString = TEXT("InventoryManager::GetItemData");
	const FItemTableRow* Row = ItemDataTable->FindRow<FItemTableRow>(ItemID, ContextString);

	if (!Row)
	{
		return false;
	}

	OutItemData = *Row;
	return true;
}

bool UInventorySubsystem::IsValidItem(FName ItemID) const
{
	if (ItemID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("UInventorySubsystem: (IsValidItem) ItemID is None."));
		return false;
	}
	
	return ItemDataCache.Contains(ItemID);
}

void UInventorySubsystem::BuildItemDataCache()
{
	ItemDataCache.Empty();
	
	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("UInventorySubsystem: ItemDataTable is null"));
		return;
	}
	
	const FString ContextString = TEXT("UInventorySubsystem::BuildItemDataCache");
	
	TArray<FName> Rownames = ItemDataTable->GetRowNames();
	
	for (const FName& RowName : Rownames)
	{
		const FItemTableRow* Row = ItemDataTable->FindRow<FItemTableRow>(RowName, ContextString);
		
		if (!Row)
		{
			continue;
		}
		
		ItemDataCache.Add(RowName, *Row);
	}
}
