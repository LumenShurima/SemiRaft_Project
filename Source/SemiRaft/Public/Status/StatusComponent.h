// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatusComponent.generated.h"

class UStatusWindow;
class UTexture2D;
class USoundBase;

UENUM(BlueprintType)
enum class EHudStatusEffect : uint8
{
	None	 UMETA(DisplayName = "None"),
	Freezing UMETA(DisplayName = "Freezing"),
	Heating	 UMETA(DisplayName = "Heating"),
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SEMIRAFT_API UStatusComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	FTimerHandle DamageTimerHandle;
	FTimerHandle StatusFadeTimerHandle;

	EHudStatusEffect ActivePersistentEffect = EHudStatusEffect::None;

	bool bDamageEffectPlaying = false;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Widget")
	TSubclassOf<UStatusWindow> StatusWindowClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStatusWindow> StatusWindow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD Effect|Texture")
	TObjectPtr<UTexture2D> FreezingTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD Effect|Texture")
	TObjectPtr<UTexture2D> HeatingTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD Effect|Texture")
	TObjectPtr<UTexture2D> TakeDamageTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD Effect|Sound")
	TObjectPtr<USoundBase> FreezingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD Effect|Sound")
	TObjectPtr<USoundBase> HeatingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD Effect|Sound")
	TObjectPtr<USoundBase> TakeDamageSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD Effect|Setting")
	float PersistentEffectOpacity = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD Effect|Setting")
	float PersistentFadeInDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD Effect|Setting")
	float PersistentFadeOutDuration = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD Effect|Setting")
	float DamageFadeInDuration = 0.06f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD Effect|Setting")
	float DamageFadeOutDuration = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HUD Effect|Setting")
	float HudEffectTickInterval = 1.0f / 60.0f;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UStatusComponent();
	
	UFUNCTION(BlueprintCallable)
	void BeginStatus(int StatusType);
	
	UFUNCTION(BlueprintCallable)
	void EndStatus(int StatusType);

	UFUNCTION(BlueprintCallable)
	void TakeDamageEffect();

private:
	EHudStatusEffect ConvertStatusType(int StatusType) const;

	UTexture2D* GetTextureByStatus(EHudStatusEffect EffectType) const;
	USoundBase* GetSoundByStatus(EHudStatusEffect EffectType) const;

	bool IsStatusImageValid() const;

	void FadePersistentEffect(float FromOpacity, float ToOpacity, float Duration);
	void ClearHudEffectTimers();
};