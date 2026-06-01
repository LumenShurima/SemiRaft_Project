// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildSystem/MakeEngineUI.h"

#include "BuildSystem/RaftActor.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "InventorySystem/InventoryComponent.h"
#include "Kismet/GameplayStatics.h"

void UMakeEngineUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (MakeButton)
	{
		MakeButton->OnClicked.AddDynamic(this, &UMakeEngineUI::OnPressMakeButton);
	}
}

void UMakeEngineUI::OnPressMakeButton()
{
	if (!InventoryComp)
	{
		return;
	}

	auto TrashItem   = InventoryComp->FindItemStacks(FName("Trash"));
	auto PlasticItem = InventoryComp->FindItemStacks(FName("Plastic"));
	
	if (TrashItem.FoundCount >= 15 && PlasticItem.FoundCount >= 15)
	{
		ARaftActor* RaftActor = Cast<ARaftActor>(
			UGameplayStatics::GetActorOfClass(this, ARaftActor::StaticClass())
		);

		if (!RaftActor || !RaftActor->InstallEngine())
		{
			return;
		}

		InventoryComp->ItemConsumption(TrashItem, 15);
		InventoryComp->ItemConsumption(PlasticItem, 15);
		UpdateItemCount();
	}
}

void UMakeEngineUI::UpdateItemCount()
{
	if (!InventoryComp || !PanjaCount || !PlasticCount)
	{
		return;
	}

	PanjaCount->SetText(FText::AsNumber(InventoryComp->FindItemStacks(FName("Trash")).FoundCount));
	PlasticCount->SetText(FText::AsNumber(InventoryComp->FindItemStacks(FName("Plastic")).FoundCount));
}
