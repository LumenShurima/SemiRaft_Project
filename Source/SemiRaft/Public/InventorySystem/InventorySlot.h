// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySystemStruct.h"
#include "InventorySlot.generated.h"


class UImage;
class UTextBlock;
class UInventoryWindow;
class UInventoryComponent;

/**
 * 
 */
UCLASS()
class SEMIRAFT_API UInventorySlot : public UUserWidget
{
	GENERATED_BODY()
	
private:
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UImage> Icon;
	
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> StackText;
	
	TObjectPtr<UInventoryComponent> InventoryComponent;
	FItem* Data = nullptr;
	
protected:
	virtual void NativeConstruct() override;
	
	virtual FReply NativeOnMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent
	) override;

	virtual void NativeOnDragDetected(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent,
		UDragDropOperation*& OutOperation
	) override;

	virtual bool NativeOnDrop(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation
	) override;

public:
	void Init(UInventoryComponent* InInventoryComponent, FItem* InItem);
	FItem* GetSlotData() const;
	void Update();
	
};


