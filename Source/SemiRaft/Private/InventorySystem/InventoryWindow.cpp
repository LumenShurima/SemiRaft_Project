// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/InventoryWindow.h"

#include "InventorySystem/InventoryComponent.h"
#include "Components/UniformGridPanel.h"
#include "InventorySystem/InventorySlot.h"
#include "GameFramework/PlayerController.h"
#include "Components/UniformGridSlot.h"
#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"


FReply UInventoryWindow::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent
)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (!TopBar)
	{
		return FReply::Unhandled();
	}

	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}
	
	const FVector2D MouseScreenPos = InMouseEvent.GetScreenSpacePosition();
	
	const FGeometry TopBarGeometry = TopBar->GetCachedGeometry();
	
	const bool bIsInsideTopBar = TopBarGeometry.IsUnderLocation(MouseScreenPos);

	if (!bIsInsideTopBar)
	{
		return FReply::Unhandled();
	}

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot);
	if (!CanvasSlot)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryWindow is not inside CanvasPanel"));
		return FReply::Unhandled();
	}

	bDraggingWindow = true;


	const FVector2D CurrentWidgetPos = CanvasSlot->GetPosition();

	const FVector2D MouseLocalPos = InGeometry.AbsoluteToLocal(MouseScreenPos);

	DragOffset = MouseLocalPos;

	UE_LOG(LogTemp, Warning, TEXT("Start Drag Inventory Window"));

	return FReply::Handled()
		.CaptureMouse(TakeWidget());
}

FReply UInventoryWindow::NativeOnMouseButtonUp(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent
)
{
	Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);

	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	if (!bDraggingWindow)
	{
		return FReply::Unhandled();
	}

	bDraggingWindow = false;

	UE_LOG(LogTemp, Warning, TEXT("End Drag Inventory Window"));

	return FReply::Handled()
		.ReleaseMouseCapture();
}

FReply UInventoryWindow::NativeOnMouseMove(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent
)
{
	Super::NativeOnMouseMove(InGeometry, InMouseEvent);

	if (!bDraggingWindow)
	{
		return FReply::Unhandled();
	}

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot);
	if (!CanvasSlot)
	{
		return FReply::Unhandled();
	}

	const FVector2D MouseScreenPos = InMouseEvent.GetScreenSpacePosition();
	
	UWidget* ParentWidget = GetParent();

	if (!ParentWidget)
	{
		return FReply::Unhandled();
	}

	const FGeometry ParentGeometry = ParentWidget->GetCachedGeometry();
	
	const FVector2D MousePosInParent = ParentGeometry.AbsoluteToLocal(MouseScreenPos);
	
	const FVector2D NewWidgetPos = MousePosInParent - DragOffset;

	CanvasSlot->SetPosition(NewWidgetPos);

	return FReply::Handled();
}
void UInventoryWindow::Init(UInventoryComponent* InInventoryComponent, APlayerController* PlayerController)
{
	if (!IsValid(InInventoryComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("UInventoryWindow: Inventory Window is not valid"));
		return;
	}
	if (!IsValid(SlotGrid))
	{
		UE_LOG(LogTemp, Warning, TEXT("UInventoryWindow: SlotGrid is not valid"));
		return;
	}
	if (!IsValid(PlayerController))
	{
		UE_LOG(LogTemp, Warning, TEXT("UInventoryWindow: PlayerController is not valid"));
		return;
	}
	if (!IsValid(InInventoryComponent->InventorySlotClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("UInventoryWindow: InventorySlotClass is not valid"));
		return;
	}
	
	

	const int32 Size = InInventoryComponent->InventorySize;
	const int32 ColumnCount = InInventoryComponent->InventoryWindowColumn;

	if ( ColumnCount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UInventoryWindow: ColumnCount is Zero"));
		return;
	}

	SlotGrid->ClearChildren();

	UE_LOG(LogTemp, Warning, TEXT("Inventory Window Created"));

	for (int32 i = 0; i < Size; ++i)
	{
		if (!InInventoryComponent->ItemArray.IsValidIndex(i))
		{
			continue;
		}

		UInventorySlot* InventorySlot = CreateWidget<UInventorySlot>(
			PlayerController,
			InInventoryComponent->InventorySlotClass
		);

		if (!InventorySlot)
		{
			continue;
		}

		const int32 Row = i / ColumnCount;
		const int32 Col = i % ColumnCount;

		UUniformGridSlot* GridSlot =
			SlotGrid->AddChildToUniformGrid(InventorySlot, Row, Col);

		if (GridSlot)
		{
			GridSlot->SetHorizontalAlignment(HAlign_Fill);
			GridSlot->SetVerticalAlignment(VAlign_Fill);
		}

		InventorySlot->Init(InInventoryComponent,&InInventoryComponent->ItemArray[i]);
		InventorySlot->Update();

		UE_LOG(LogTemp, Warning, TEXT("Inventory Slot Created: Index=%d Row=%d Col=%d"), i, Row, Col);
	}
}
