// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Class.h"
#include "ItemData.generated.h"




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
	FName ItemID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ItemName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType ItemType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Stack;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStack = 1;
};
