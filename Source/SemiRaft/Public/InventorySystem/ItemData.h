// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Class.h"
#include "ItemData.generated.h"


class UTexture2D;

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
struct SEMIRAFT_API FItemData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Index = INDEX_NONE;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ItemName = FText::GetEmpty();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> ItemIcon = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType ItemType = EItemType::Misc;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Stack = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStack = 1;
};
