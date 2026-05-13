// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/InventoryComponent.h"
#include "InventorySystem/InventorySystemWidget.h"
#include "InventorySystem/InventorySubsystem.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	ItemArray.SetNum(InventorySize);
	
	for (int i = 0; i < InventorySize; i++)
	{
		ItemArray[i].Index = i; 
	}
	
	// ...
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// ...
	
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UInventoryComponent::CreateInventoryWidget(APlayerController* PlayerController)
{
	if (!IsValid(PlayerController))
	{
		UE_LOG(LogTemp, Error, TEXT("UInventoryComponent: PlayerController is invalid"));
		return;
	}

	if (!InventorySystemWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("UInventoryComponent: InventorySystemWidgetClass is null"));
		return;
	}

	if (IsValid(InventorySystemWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent: InventorySystemWidget already exists"));
		return;
	}

	InventorySystemWidget = CreateWidget<UInventorySystemWidget>(
		PlayerController,
		InventorySystemWidgetClass
	);

	if (!IsValid(InventorySystemWidget))
	{
		UE_LOG(LogTemp, Error, TEXT("UInventoryComponent: Failed to create InventorySystemWidget"));
		return;
	}

	InventorySystemWidget->Init(this, PlayerController);
	InventorySystemWidget->AddToViewport();

	PlayerController->SetShowMouseCursor(true);

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(InventorySystemWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PlayerController->SetInputMode(InputMode);

	UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent: InventorySystemWidget Created"));
}


void UInventoryComponent::DestroyInventoryWidget()
{
	if (!IsValid(InventorySystemWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent: InventorySystemWidget is already null"));
		return;
	}
	
	InventorySystemWidget->RemoveFromParent();
	InventorySystemWidget = nullptr;

	APlayerController* PlayerController = nullptr;
	
	if (AActor* Owner = GetOwner())
	{
		if (APawn* PawnOwner = Cast<APawn>(Owner))
		{
			PlayerController = Cast<APlayerController>(PawnOwner->GetController());
		}
	}

	if (IsValid(PlayerController))
	{
		PlayerController->SetShowMouseCursor(false);
		PlayerController->SetInputMode(FInputModeGameOnly());
	}

	UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent: InventorySystemWidget Removed"));
}

void UInventoryComponent::DragDropProcess(int FromIndex, int ToIndex)
{
	if (FromIndex == ToIndex)
	{
		return;
	}
	FItem* From = &ItemArray[FromIndex];
	FItem* To = &ItemArray[ToIndex];
	UInventorySubsystem* InventorySubsystem = UInventorySubsystem::Get(this);
	if (!InventorySubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("UInventoryComponent: (DragDropProcess) InventorySubsystem is null"));
		return;
	}
	
	if (From->ItemID == To->ItemID)
	{
		FItemTableRow ItemData;
		if (!InventorySubsystem->GetItemData(To->ItemID, ItemData))
		{
			UE_LOG(LogTemp, Error, TEXT("UInventoryComponent: (DragDropProcess) InventorySubsystem Get Item Data Failed"));
			return;
		}
		
		int MaxStack = ItemData.MaxStack;
		
		int SumStack = To->Stack + From->Stack;
		if (SumStack > MaxStack)
		{
			To->Stack = MaxStack;
			From->Stack = SumStack - MaxStack;
		}
		else
		{
			To->Stack = SumStack;
			From->Stack = 0;
			From->ItemID = NAME_None;
		}
	}
	else
	{
		ItemArray.Swap(FromIndex, ToIndex);
		ItemArray[FromIndex].Index = FromIndex;
		ItemArray[ToIndex].Index = ToIndex;
	}
	
}

