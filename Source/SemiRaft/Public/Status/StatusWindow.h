// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StatusWindow.generated.h"


class UImage;
/**
 * 
 */
UCLASS()
class SEMIRAFT_API UStatusWindow : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UImage> StatusEffect;
	
	// UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true"))
	// TObjectPtr<UImage> DamageEffect;

	
};
