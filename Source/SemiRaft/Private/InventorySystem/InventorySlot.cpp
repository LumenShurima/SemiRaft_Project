// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/InventorySlot.h"
#include "InventorySystem/InventoryWindow.h"
#include "InventorySystem/InventoryComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/DragDropOperation.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/PanelWidget.h"
#include "Engine/Texture2D.h"
#include "InventorySystem/InventorySubsystem.h"

void UInventorySlot::NativeConstruct()
{
	Super::NativeConstruct();
}

FReply UInventorySlot::NativeOnMouseButtonDown(
	const FGeometry& InGeometry, 
	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		UE_LOG(LogTemp, Log, TEXT("UInventorySlot::NativeOnMouseButtonDown"));
		if (!Data->ItemID.IsNone())
		{
			return UWidgetBlueprintLibrary::DetectDragIfPressed(
				InMouseEvent,
				this,
				EKeys::LeftMouseButton).NativeReply;
		}
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UInventorySlot::NativeOnDragDetected(
	const FGeometry& InGeometry, 
	const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	
	UDragDropOperation* DragOperation =
		UWidgetBlueprintLibrary::CreateDragDropOperation(
			UDragDropOperation::StaticClass()
		);
	
	DragOperation->Payload = this;
	DragOperation->DefaultDragVisual = this;
	DragOperation->Pivot = EDragPivot::BottomRight;

	
	UE_LOG(LogTemp, Log, TEXT("UInventorySlot::NativeOnDragDetected"));
	
	OutOperation = DragOperation;
}

bool UInventorySlot::NativeOnDrop(
	const FGeometry& InGeometry, 
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	if (!InOperation)
	{
		return false;
	}

	UInventorySlot* FromSlot = Cast<UInventorySlot>(InOperation->Payload);

	if (!FromSlot || FromSlot == this)
	{
		return false;
	}

	if (!InventoryComponent)
	{
		return false;
	}

	FItem* FromData = FromSlot->GetSlotData();
	FItem* ToData = GetSlotData();

	if (!FromData || !ToData)
	{
		return false;
	}
	
	int FromIndex = FromData->Index;
	int ToIndex = ToData->Index;
	
	InventoryComponent->DragDropProcess(FromIndex, ToIndex);
	
	FromSlot->Update();
	Update();
	
	return true;
}

void UInventorySlot::Init(UInventoryComponent* InInventoryComponent, FItem* InItem)
{
	InventoryComponent = InInventoryComponent;
	Data = InItem;
	Update();
}

FItem* UInventorySlot::GetSlotData() const { return Data; }

void UInventorySlot::Update()
{

	if (StackText)
	{
		StackText->SetText((Data->Stack <= 0 ? FText::GetEmpty() : FText::AsNumber(Data->Stack)));
	}

	if (Icon)
	{
		auto InventorySubSystem = UInventorySubsystem::Get(this);
		FItemTableRow Item;
		if (!InventorySubSystem->GetItemData(Data->ItemID, Item))
		{
			UE_LOG(LogTemp, Warning, TEXT("UInventorySlot: Failed Get SubSystem ItemData"))
			return;
		}
		Icon->SetBrushFromTexture(Item.Icon, true);
	}
	
}



