// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InventorySystemStruct.h"
#include "InventorySubsystem.generated.h"


class UDataTable;
class UTexture2D;
class AItemBase;

/**
 * 
 */
UCLASS()
class SEMIRAFT_API UInventorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
private:
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UDataTable> ItemDataTable;
	
	
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	virtual void Deinitialize() override;
	
	static UInventorySubsystem* Get(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable)
	bool GetItemData(FName ItemID, FItemTableRow& OutItemData) const;
	
	UFUNCTION(BlueprintCallable)
	bool IsValidItem(FName ItemID) const;
	
	UFUNCTION(BlueprintCallable)
	void CreateWorldItem(TSubclassOf<AItemBase> ItemClass, FItem InData, FTransform InTransform);
	
	FItem DeleteWorldItem(AItemBase* TargetItem);
	
private:
	// 캐시
	UPROPERTY()
	TMap<FName, FItemTableRow> ItemDataCache;
	
private:
	void BuildItemDataCache();
	
	
};
