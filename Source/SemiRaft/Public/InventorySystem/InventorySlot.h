// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySlot.generated.h"


class UTexture2D;
class UImage;
class UTextBlock;

USTRUCT(BlueprintType)
struct FInventorySlotData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FName ItemID;
	
	UPROPERTY(BlueprintReadWrite)
	UTexture2D* ItemTexture;

	UPROPERTY(BlueprintReadWrite)
	int InventoryIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite)
	int Stack = 0;
};
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
	
	FInventorySlotData Data;
	
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
	void SetSlotData(const FInventorySlotData& InData);
	const FInventorySlotData& GetSlotData() const;
};


