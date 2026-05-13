// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/InventorySystemWidget.h"

#include "ShaderPrintParameters.h"
#include "Components/CanvasPanelSlot.h"
#include "InventorySystem/InventoryComponent.h"
#include "InventorySystem/InventoryWindow.h"
#include "Components/CanvasPanel.h"

void UInventorySystemWidget::InitInventoryWindow(UInventoryComponent* InComponent)
{
	if (BP_InventoryWindow != nullptr && InComponent)
	{
		BP_InventoryWindow->InventoryComponent = InComponent;
		
		auto InventoryWindow = Cast<UCanvasPanelSlot>(BP_InventoryWindow->Slot);
		if (IsValid(InventoryWindow))
		{
			
			if (InComponent->InventoryWindowColumn <= 0)
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid InventoryWindowColumn"));
				return;
			}
			int Column = InComponent->InventoryWindowColumn;
			int Size = InComponent->InventorySize;
			int Row = Size / Column;
			
			
			FVector2D TargetSize2D = FVector2D(Column*100, Row*100+((Row*100) * 0.1));
			InventoryWindow->SetSize(TargetSize2D);
			UE_LOG(LogTemp, Warning, TEXT("Window Size X=%f,, Y=%f"),TargetSize2D.X, TargetSize2D.Y);
			BP_InventoryWindow->CreateInventorySlot();
		}
	}
}