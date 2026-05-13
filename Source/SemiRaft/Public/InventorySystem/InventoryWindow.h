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
	
public:
	UPROPERTY(meta = (BindWidget))
	UUniformGridPanel* SlotGrid;
	
	UPROPERTY(meta = (BindWidget))
	UBorder* TopBar;
	
	UInventoryComponent* InventoryComponent = nullptr;
	
	
	void CreateInventorySlot();
	

	
	
	
};
