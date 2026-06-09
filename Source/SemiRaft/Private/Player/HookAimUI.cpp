// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/HookAimUI.h"

#include "Components/Image.h"

void UHookAimUI::UpdatePercent(double Percent)
{
	if (!RoundProgressbarInst)
	{
		if (!ParentMaterial)
		{
			UE_LOG(LogTemp, Warning, TEXT("ParentMaterial이 지정되지 않았습니다!"));
			return;
		}
		
		RoundProgressbarInst = UMaterialInstanceDynamic::Create(ParentMaterial, this);
        
		if (RoundProgressbarInst && PercentCircle)
		{
			PercentCircle->SetBrushFromMaterial(RoundProgressbarInst);
		}
	}
	
	if (RoundProgressbarInst)
	{
		RoundProgressbarInst->SetScalarParameterValue(TEXT("Percent"), static_cast<float>(Percent));
	}
}

void UHookAimUI::UpdateCurrentItemUI(int32 numKey)
{
	UE_LOG(LogTemp, Warning, TEXT("UpdateCurrentItemUI: %d"), numKey);
	
	ImageHook->SetVisibility(ESlateVisibility::Visible);
	ImageHammer->SetVisibility(ESlateVisibility::Visible);
	ImageAxe->SetVisibility(ESlateVisibility::Visible);
	ImageSpear->SetVisibility(ESlateVisibility::Visible);
	
	switch (numKey)
	{
		case 1:
		ImageHook->SetVisibility(ESlateVisibility::Hidden);
		break;
		case 2:
		ImageHammer->SetVisibility(ESlateVisibility::Hidden);
		break;
		case 3:
		ImageAxe->SetVisibility(ESlateVisibility::Hidden);
		break;
		case 4:
		ImageSpear->SetVisibility(ESlateVisibility::Hidden);
		break;
	}	
	
}
