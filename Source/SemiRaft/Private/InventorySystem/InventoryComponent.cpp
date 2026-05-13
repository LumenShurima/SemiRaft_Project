// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/InventoryComponent.h"
#include "InventorySystem/InventorySystemWidget.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	InventoryArray.SetNum(InventorySize);

	for (int32 i = 0; i < InventoryArray.Num(); ++i)
	{
		InventoryArray[i].Index = i;
	}

	// ...
	
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UInventoryComponent::CreateInventoryWidget()
{
	PC = Cast<APlayerController>(GetOwner());
	if (IsValid(PC))
	{
		
		InventorySystemWidget = CreateWidget<UInventorySystemWidget>(
		PC,
		InventorySystemWidgetClass);

		if (InventorySystemWidget)
		{
			
			InventorySystemWidget->InitInventoryWindow(this);
			InventorySystemWidget->SetVisibility(ESlateVisibility::Hidden);
			InventorySystemWidget->AddToViewport();
			UE_LOG(LogTemp, Warning, TEXT("InventorySystemWidget Created"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("InventorySystemWidget is null"));
		}
	}
	
}

void UInventoryComponent::EnableInventoryWidget()
{
	if (InventorySystemWidget != nullptr && PC != nullptr)
	{
		PC->SetShowMouseCursor(true);
		InventorySystemWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void UInventoryComponent::DisableInventoryWidget()
{
	if (InventorySystemWidget != nullptr && PC != nullptr)
	{
		PC->SetShowMouseCursor(false);
		InventorySystemWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

