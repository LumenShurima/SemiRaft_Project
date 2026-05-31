// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MakeEngineUI.generated.h"

/**
 * 
 */
UCLASS()
class SEMIRAFT_API UMakeEngineUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> MakeButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> PanjaCount;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> PlasticCount;
	
	UPROPERTY()
	TObjectPtr<class UInventoryComponent> InventoryComp;
	
	/* 엔진BP 만들어서 넣기
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class >
	*/ 
	
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void OnPressMakeButton();
	void UpdateItemCount();
};
