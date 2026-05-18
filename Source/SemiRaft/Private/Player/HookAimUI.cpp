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
