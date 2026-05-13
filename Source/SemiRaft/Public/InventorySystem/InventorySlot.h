// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemData.h"


#include "InventorySlot.generated.h"


class UImage;
class UTextBlock;
class UInventoryWindow;

/**
 * 
 */
UCLASS()
class SEMIRAFT_API UInventorySlot : public UUserWidget
{
	GENERATED_BODY()
	
private:
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"))
	UImage* Icon;
	
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"))
	UTextBlock* StackText;
	
	
	TObjectPtr<UInventoryWindow> InventoryWindow;
	
	FItemData* Data = nullptr;
	
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
	
private:
	
	
	
public:
	void InitSlot(UInventoryWindow* InInventoryWindow);
	void SetSlotData(FItemData* InData);
	FItemData* GetSlotData() const;
	void UpdateSlot();
	
};


