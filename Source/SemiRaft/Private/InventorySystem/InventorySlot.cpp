// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/InventorySlot.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/DragDropOperation.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UInventorySlot::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (StackText)
	{
		StackText->SetText(FText::FromString(TEXT("0")));
	}
}

FReply UInventorySlot::NativeOnMouseButtonDown(
	const FGeometry& InGeometry, 
	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		UE_LOG(LogTemp, Log, TEXT("UInventorySlot::NativeOnMouseButtonDown"));
		return UWidgetBlueprintLibrary::DetectDragIfPressed(
			InMouseEvent,
			this,
			EKeys::LeftMouseButton).NativeReply;
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
	DragOperation->Pivot = EDragPivot::MouseDown;
	
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
	
	

	const FInventorySlotData FromData = FromSlot->GetSlotData();
	const FInventorySlotData ToData = GetSlotData();
	
	
	UE_LOG(
		LogTemp,
		Log,
		TEXT("Drop: ItemID=%s Index=%d Stack=%d"),
		*FromData.ItemID.ToString(),
		FromData.InventoryIndex,
		FromData.Stack
	);

	FromSlot->SetSlotData(ToData);
	SetSlotData(FromData);

	return true;
}

void UInventorySlot::SetSlotData(const FInventorySlotData& InData)
{
	Data = InData;

	if (StackText)
	{
		StackText->SetText(FText::AsNumber(Data.Stack));
	}

	if (Icon && InData.ItemTexture)
	{
		Icon->SetBrushFromTexture(InData.ItemTexture, true);
	}
}

const FInventorySlotData& UInventorySlot::GetSlotData() const
{
	return Data;
}



