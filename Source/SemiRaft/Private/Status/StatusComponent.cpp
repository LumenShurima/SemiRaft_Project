// Fill out your copyright notice in the Description page of Project Settings.

#include "Status/StatusComponent.h"

#include "Status/StatusWindow.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void UStatusComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!StatusWindowClass)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: StatusWindowClass is Invalid."),
			*GetClass()->GetName(),
			TEXT(__FUNCTION__));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: World is Invalid."),
			*GetClass()->GetName(),
			TEXT(__FUNCTION__));
		return;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: PlayerController is Invalid."),
			*GetClass()->GetName(),
			TEXT(__FUNCTION__));
		return;
	}

	StatusWindow = CreateWidget<UStatusWindow>(PlayerController, StatusWindowClass);
	if (!StatusWindow)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Failed to Create StatusWindow."),
			*GetClass()->GetName(),
			TEXT(__FUNCTION__));
		return;
	}

	StatusWindow->AddToViewport(-10);
	StatusWindow->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	if (!StatusWindow->StatusEffect)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: StatusEffect is Invalid. Check BindWidget or Widget Image name."),
			*GetClass()->GetName(),
			TEXT(__FUNCTION__));
		return;
	}

	StatusWindow->StatusEffect->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	StatusWindow->StatusEffect->SetRenderOpacity(0.0f);

	UE_LOG(LogTemp, Warning, TEXT("%s::%s: StatusWindow Created."),
		*GetClass()->GetName(),
		TEXT(__FUNCTION__));
}

void UStatusComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

UStatusComponent::UStatusComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

EHudStatusEffect UStatusComponent::ConvertStatusType(int StatusType) const
{
	// 원하는 값으로 바꿔도 됨.
	// 0 = Freeze
	// 1 = Heat
	switch (StatusType)
	{
	case 0:
		return EHudStatusEffect::Freezing;

	case 1:
		return EHudStatusEffect::Heating;

	default:
		return EHudStatusEffect::None;
	}
}

UTexture2D* UStatusComponent::GetTextureByStatus(EHudStatusEffect EffectType) const
{
	switch (EffectType)
	{
	case EHudStatusEffect::Freezing:
		return FreezingTexture;

	case EHudStatusEffect::Heating:
		return HeatingTexture;

	default:
		return nullptr;
	}
}

USoundBase* UStatusComponent::GetSoundByStatus(EHudStatusEffect EffectType) const
{
	switch (EffectType)
	{
	case EHudStatusEffect::Freezing:
		return FreezingSound;

	case EHudStatusEffect::Heating:
		return HeatingSound;

	default:
		return nullptr;
	}
}

bool UStatusComponent::IsStatusImageValid() const
{
	return StatusWindow && StatusWindow->StatusEffect;
}

void UStatusComponent::BeginStatus(int StatusType)
{
	const EHudStatusEffect EffectType = ConvertStatusType(StatusType);

	if (EffectType == EHudStatusEffect::None)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s::%s: Invalid StatusType: %d"),
			*GetClass()->GetName(),
			TEXT(__FUNCTION__),
			StatusType);
		return;
	}

	// 이미 같은 상태면 다시 페이드 시작하지 않음.
	// 온도 조건 체크에서 매 프레임 BeginStatus가 호출될 수 있으므로 중요함.
	if (ActivePersistentEffect == EffectType)
	{
		return;
	}

	if (!IsStatusImageValid())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: StatusWindow or StatusEffect is Invalid."),
			*GetClass()->GetName(),
			TEXT(__FUNCTION__));
		return;
	}

	UTexture2D* EffectTexture = GetTextureByStatus(EffectType);
	if (!EffectTexture)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: EffectTexture is Invalid."),
			*GetClass()->GetName(),
			TEXT(__FUNCTION__));
		return;
	}

	ActivePersistentEffect = EffectType;

	USoundBase* EffectSound = GetSoundByStatus(EffectType);
	if (EffectSound)
	{
		UGameplayStatics::PlaySound2D(this, EffectSound);
	}

	// 데미지 이펙트가 재생 중이면 상태만 저장.
	// 데미지 끝난 뒤 TakeDamageEffect 쪽에서 다시 페이드 인 처리함.
	if (bDamageEffectPlaying)
	{
		return;
	}

	UImage* EffectImage = StatusWindow->StatusEffect;

	EffectImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	EffectImage->SetBrushFromTexture(EffectTexture);

	const float CurrentOpacity = EffectImage->GetRenderOpacity();

	FadePersistentEffect(
		CurrentOpacity,
		PersistentEffectOpacity,
		PersistentFadeInDuration
	);
}
void UStatusComponent::EndStatus(int StatusType)
{
	const EHudStatusEffect EffectType = ConvertStatusType(StatusType);

	if (EffectType == EHudStatusEffect::None)
	{
		return;
	}

	// 현재 켜진 지속형 상태와 다른 종료 요청이면 무시.
	if (ActivePersistentEffect != EffectType)
	{
		return;
	}

	ActivePersistentEffect = EHudStatusEffect::None;

	if (!IsStatusImageValid())
	{
		return;
	}

	// 데미지 이펙트가 재생 중이면 데미지 끝난 뒤 0으로 정리됨.
	if (bDamageEffectPlaying)
	{
		return;
	}

	UImage* EffectImage = StatusWindow->StatusEffect;

	const float CurrentOpacity = EffectImage->GetRenderOpacity();

	FadePersistentEffect(
		CurrentOpacity,
		0.0f,
		PersistentFadeOutDuration
	);
}
void UStatusComponent::FadePersistentEffect(float FromOpacity, float ToOpacity, float Duration)
{
	UWorld* World = GetWorld();
	if (!World || !IsStatusImageValid())
	{
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();
	TimerManager.ClearTimer(StatusFadeTimerHandle);

	TWeakObjectPtr<UImage> EffectImageWeak = StatusWindow->StatusEffect;

	float Elapsed = 0.0f;

	TimerManager.SetTimer(
		StatusFadeTimerHandle,
		FTimerDelegate::CreateWeakLambda(
			this,
			[this, EffectImageWeak, FromOpacity, ToOpacity, Duration, Elapsed]() mutable
			{
				UWorld* InnerWorld = GetWorld();
				if (!InnerWorld)
				{
					return;
				}

				UImage* EffectImage = EffectImageWeak.Get();
				if (!EffectImage)
				{
					InnerWorld->GetTimerManager().ClearTimer(StatusFadeTimerHandle);
					return;
				}

				const float SafeDuration = FMath::Max(Duration, KINDA_SMALL_NUMBER);

				Elapsed += InnerWorld->GetDeltaSeconds();

				const float Alpha = FMath::Clamp(Elapsed / SafeDuration, 0.0f, 1.0f);
				const float NewOpacity = FMath::Lerp(FromOpacity, ToOpacity, Alpha);

				EffectImage->SetRenderOpacity(NewOpacity);

				if (Alpha >= 1.0f)
				{
					InnerWorld->GetTimerManager().ClearTimer(StatusFadeTimerHandle);
					EffectImage->SetRenderOpacity(ToOpacity);
				}
			}
		),
		HudEffectTickInterval,
		true
	);
}

void UStatusComponent::TakeDamageEffect()
{
	if (!IsStatusImageValid())
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: StatusWindow or StatusEffect is Invalid."),
			*GetClass()->GetName(),
			TEXT(__FUNCTION__));
		return;
	}

	if (!TakeDamageTexture)
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: TakeDamageTexture is Invalid."),
			*GetClass()->GetName(),
			TEXT(__FUNCTION__));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();

	TimerManager.ClearTimer(DamageTimerHandle);
	TimerManager.ClearTimer(StatusFadeTimerHandle);

	bDamageEffectPlaying = true;

	UImage* EffectImage = StatusWindow->StatusEffect;
	EffectImage->SetBrushFromTexture(TakeDamageTexture);
	EffectImage->SetRenderOpacity(0.0f);

	if (TakeDamageSound)
	{
		UGameplayStatics::PlaySound2D(this, TakeDamageSound);
	}

	TWeakObjectPtr<UImage> EffectImageWeak = EffectImage;

	const float TotalDuration = DamageFadeInDuration + DamageFadeOutDuration;

	float Elapsed = 0.0f;

	TimerManager.SetTimer(
		DamageTimerHandle,
		FTimerDelegate::CreateWeakLambda(this,
			[this, EffectImageWeak, TotalDuration, Elapsed]() mutable
			{
				UWorld* InnerWorld = GetWorld();
				if (!InnerWorld)
				{
					return;
				}

				UImage* EffectImage = EffectImageWeak.Get();
				if (!EffectImage)
				{
					bDamageEffectPlaying = false;
					InnerWorld->GetTimerManager().ClearTimer(DamageTimerHandle);
					return;
				}

				Elapsed += InnerWorld->GetDeltaSeconds();

				float NewOpacity = 0.0f;

				if (Elapsed <= DamageFadeInDuration)
				{
					const float SafeFadeIn = FMath::Max(DamageFadeInDuration, KINDA_SMALL_NUMBER);
					const float Alpha = FMath::Clamp(Elapsed / SafeFadeIn, 0.0f, 1.0f);

					NewOpacity = Alpha;
				}
				else
				{
					const float FadeOutElapsed = Elapsed - DamageFadeInDuration;
					const float SafeFadeOut = FMath::Max(DamageFadeOutDuration, KINDA_SMALL_NUMBER);
					const float Alpha = FMath::Clamp(FadeOutElapsed / SafeFadeOut, 0.0f, 1.0f);

					NewOpacity = 1.0f - Alpha;
				}

				EffectImage->SetRenderOpacity(NewOpacity);

				if (Elapsed >= TotalDuration)
				{
					InnerWorld->GetTimerManager().ClearTimer(DamageTimerHandle);

					bDamageEffectPlaying = false;

					// 데미지 HUD 종료 후에도 Heat / Freeze 상태가 유지 중이면 복귀.
					if (ActivePersistentEffect != EHudStatusEffect::None)
					{
						UTexture2D* PersistentTexture = GetTextureByStatus(ActivePersistentEffect);

						if (PersistentTexture)
						{
							EffectImage->SetBrushFromTexture(PersistentTexture);
							EffectImage->SetRenderOpacity(0.0f);

							FadePersistentEffect(
								0.0f,
								PersistentEffectOpacity,
								PersistentFadeInDuration
							);

							return;
						}
					}

					EffectImage->SetRenderOpacity(0.0f);
				}
			}
		),
		HudEffectTickInterval,
		true
	);
}

void UStatusComponent::ClearHudEffectTimers()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();

	TimerManager.ClearTimer(DamageTimerHandle);
	TimerManager.ClearTimer(StatusFadeTimerHandle);

	bDamageEffectPlaying = false;
}