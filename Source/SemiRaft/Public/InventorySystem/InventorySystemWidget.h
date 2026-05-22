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
	TObjectPtr<UCanvasPanel> RootWindow;
	
	
public:
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UInventoryWindow> InventoryWindow;

public:
	void Init(UInventoryComponent* InComponent, APlayerController* PlayerController);
	
	void  Update();

	
};
