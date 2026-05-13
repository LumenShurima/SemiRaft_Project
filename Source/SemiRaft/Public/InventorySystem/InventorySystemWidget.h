// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySystemWidget.generated.h"


class UInventoryWindow;
class UInventoryComponent;
class UCanvasPanel;
/**
 * 
 */
UCLASS()
class SEMIRAFT_API UInventorySystemWidget : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"))
	UCanvasPanel* RootWindow;
	
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"))
	UInventoryWindow* BP_InventoryWindow;

	
public:
	void Init(UInventoryComponent* InComponent, APlayerController* PlayerController);
	

	
};
