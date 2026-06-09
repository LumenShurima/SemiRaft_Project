// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HookAimUI.generated.h"

/**
 * 
 */
UCLASS()
class SEMIRAFT_API UHookAimUI : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UImage> PercentCircle;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UImage> ImageHook;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UImage> ImageHammer;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UImage> ImageAxe;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UImage> ImageSpear;
	
	UPROPERTY(EditDefaultsOnly, Category = "MyVar")
	UMaterialInterface* ParentMaterial;
	
	UPROPERTY()
	UMaterialInstanceDynamic* RoundProgressbarInst;
	

	void UpdatePercent(double Percent);
	void UpdateCurrentItemUI(int32 numKey);
};
