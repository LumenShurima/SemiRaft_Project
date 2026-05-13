// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventorySystemStruct.h"
#include "InventoryComponent.generated.h"

class UInventorySystemWidget;
class APlayerController;
class UInventorySlot;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class SEMIRAFT_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int InventorySize = 20;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int InventoryWindowColumn = 5;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FItem> ItemArray;
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UInventorySystemWidget> InventorySystemWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UInventorySlot> InventorySlotClass;
	
	UPROPERTY()
	TObjectPtr<UInventorySystemWidget> InventorySystemWidget = nullptr;
	
	
	
public:	
	// Sets default values for this component's properties
	UInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void CreateInventoryWidget(APlayerController* PlayerController);
	
	UFUNCTION(BlueprintCallable)
	void DestroyInventoryWidget();
	
	void DragDropProcess(int FromIndex, int ToIndex);
};

