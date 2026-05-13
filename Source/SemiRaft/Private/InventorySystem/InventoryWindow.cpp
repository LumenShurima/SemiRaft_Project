// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/InventoryWindow.h"

#include "InventorySystem/InventoryComponent.h"
#include "Components/UniformGridPanel.h"
#include "InventorySystem/InventorySlot.h"
#include "GameFramework/PlayerController.h"
#include "Components/UniformGridSlot.h"
#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"

void UInventoryWindow::CreateInventorySlot()
{
	if (!InventoryComponent || !SlotGrid)
	{
		return;
	}

	const int32 Size = InventoryComponent->InventorySize;
	const int32 ColumnCount = InventoryComponent->InventoryWindowColumn;
	APlayerController* PC = InventoryComponent->PC;

	if (!PC || !InventoryComponent->InventorySlotClass || ColumnCount <= 0)
	{
		return;
	}

	SlotGrid->ClearChildren();

	UE_LOG(LogTemp, Warning, TEXT("Inventory Window Created"));

	for (int32 i = 0; i < Size; ++i)
	{
		if (!InventoryComponent->InventoryArray.IsValidIndex(i))
		{
			continue;
		}

		UInventorySlot* InventorySlot = CreateWidget<UInventorySlot>(
			PC,
			InventoryComponent->InventorySlotClass
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

		InventorySlot->InitSlot(this);
		InventorySlot->SetSlotData(&InventoryComponent->InventoryArray[i]);
		InventorySlot->UpdateSlot();

		UE_LOG(LogTemp, Warning, TEXT("Inventory Slot Created: Index=%d Row=%d Col=%d"), i, Row, Col);
	}
}
