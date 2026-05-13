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

void UInventorySlot::NativeConstruct()
{
	Super::NativeConstruct();
	
	
	UpdateSlot();
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
	DragOperation->Pivot = EDragPivot::CenterCenter;
	
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

	if (!InventoryWindow || !InventoryWindow->InventoryComponent)
	{
		return false;
	}

	FItemData* FromData = FromSlot->GetSlotData();
	FItemData* ToData = GetSlotData();

	if (!FromData || !ToData)
	{
		return false;
	}

	TArray<FItemData>& InventoryArray =
		InventoryWindow->InventoryComponent->InventoryArray;

	if (!InventoryArray.IsValidIndex(FromData->Index) ||
		!InventoryArray.IsValidIndex(ToData->Index))
	{
		return false;
	}

	const int32 FromIndex = FromData->Index;
	const int32 ToIndex = ToData->Index;

	InventoryArray.Swap(FromIndex, ToIndex);

	InventoryArray[FromIndex].Index = FromIndex;
	InventoryArray[ToIndex].Index = ToIndex;
	
	UpdateSlot();
	
	return true;
}

void UInventorySlot::InitSlot(UInventoryWindow* InInventoryWindow)
{
	InventoryWindow = InInventoryWindow;
}

void UInventorySlot::SetSlotData(FItemData* InData) { Data = InData; }

FItemData* UInventorySlot::GetSlotData() const { return Data; }

void UInventorySlot::UpdateSlot()
{
	if (StackText)
	{
		StackText->SetText((Data->Stack <= 0 ? FText::GetEmpty() : FText::AsNumber(Data->Stack)));
	}

	if (Icon)
	{
		Icon->SetBrushFromTexture(Data->ItemIcon, true);
	}
}



