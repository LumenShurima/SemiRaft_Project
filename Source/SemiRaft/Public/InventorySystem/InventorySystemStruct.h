// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InventorySystemStruct.generated.h"

class ItemBase;

UENUM( BlueprintType )
enum class EItemType : uint8
{
	Material,
	Tool,
	Weapon,
	Misc,
	End
};
/**
 * 
 */
USTRUCT(BlueprintType)
struct SEMIRAFT_API FItemTableRow : public FTableRowBase
{
	GENERATED_BODY()
	
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// TSubclassOf<ItemBase> ItemClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> Icon = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType ItemType = EItemType::Misc;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStack = 1;
	
};
/**
 * 
 */
USTRUCT(BlueprintType)
struct FItem
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName ItemID = NAME_None;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Index = 0;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Stack = 0;
};
