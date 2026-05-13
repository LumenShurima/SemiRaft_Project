// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWindow.generated.h"


class UInventoryComponent;
class UUniformGridPanel;
class UBorder;

/**
 * 
 */
UCLASS()
class SEMIRAFT_API UInventoryWindow : public UUserWidget
{
	GENERATED_BODY()
private:
	bool bDraggingWindow = false;

	FVector2D DragOffset = FVector2D::ZeroVector;
	
public:
	UPROPERTY(meta = (BindWidget))
	UUniformGridPanel* SlotGrid;
	
	UPROPERTY(meta = (BindWidget))
	UBorder* TopBar;
	
	virtual FReply NativeOnMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent
	) override;

	virtual FReply NativeOnMouseButtonUp(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent
	) override;
	
	virtual FReply NativeOnMouseMove(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent
	) override;
	
	void Init(UInventoryComponent* InInventoryComponent, APlayerController* PlayerController);
};
