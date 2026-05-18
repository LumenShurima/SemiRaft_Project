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
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UImage> PercentCircle;
	
	UPROPERTY(EditDefaultsOnly, Category = "MyVar")
	UMaterialInterface* ParentMaterial;
	
	UPROPERTY()
	UMaterialInstanceDynamic* RoundProgressbarInst;
	
public:
	void UpdatePercent(double Percent);
};
