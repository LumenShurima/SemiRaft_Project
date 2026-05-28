// Fill out your copyright notice in the Description page of Project Settings.


#include "Weather/WeatherVolumetricCloud.h"

#include "FrameTypes.h"
#include "NiagaraActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"


AWeatherVolumetricCloud::AWeatherVolumetricCloud()
{
	
}

void AWeatherVolumetricCloud::BeginPlay()
{
	Super::BeginPlay();
	
	VolumetricCloudComponentProxy = GetComponentByClass<UVolumetricCloudComponent>();
	if (VolumetricCloudComponentProxy)
	{
		CloudDynamicMaterial = UMaterialInstanceDynamic::Create(VolumetricCloudComponentProxy->GetMaterial(), this);
		VolumetricCloudComponentProxy->SetMaterial(CloudDynamicMaterial);
		
	}
}

void AWeatherVolumetricCloud::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	
	
	if (bStateToTargetState)
	{
		if (!CloudDynamicMaterial)
		{
			UE_LOG(LogTemp, Error, TEXT("%s::%s: CloudDynamicMaterial is Not Valid"),
						*GetClass()->GetName(), TEXT(__FUNCTION__));

			bStateToTargetState = false;
			ElapsedDuration = 0.0f;
		}
		else
		{
			const float SafeDuration = FMath::Max(LerpDuration, KINDA_SMALL_NUMBER);

			ElapsedDuration += DeltaTime;

			const float Alpha = FMath::Clamp(ElapsedDuration / SafeDuration, 0.0f, 1.0f);
			
			
			StormClouds				= FMath::Lerp(CurrentSnapshot.StormClouds				, TargetSnapshot.StormClouds, Alpha);
			Storm_LightningTexScale = FMath::Lerp(CurrentSnapshot.Storm_LightningTexScale	, TargetSnapshot.Storm_LightningTexScale, Alpha);
			Storm_LightningAnim		= FMath::Lerp(CurrentSnapshot.Storm_LightningAnim		, TargetSnapshot.Storm_LightningAnim, Alpha);
			Storm_LightningClouds	= FMath::Lerp(CurrentSnapshot.Storm_LightningClouds		, TargetSnapshot.Storm_LightningClouds, Alpha);
			Storm_LightningColor	= FMath::Lerp(CurrentSnapshot.Storm_LightningColor		, TargetSnapshot.Storm_LightningColor, Alpha);
			Storm_LightningMasks	= FMath::Lerp(CurrentSnapshot.Storm_LightningMasks		, TargetSnapshot.Storm_LightningMasks, Alpha);
			Storm_AlbedoColor		= FMath::Lerp(CurrentSnapshot.Storm_AlbedoColor			, TargetSnapshot.Storm_AlbedoColor, Alpha);
			

			if (Alpha >= 1.0f)
			{
				bStateToTargetState = false;
				ElapsedDuration = 0.0f;

				// 최종값 보정
				StormClouds				= TargetSnapshot.StormClouds;
				Storm_LightningTexScale = TargetSnapshot.Storm_LightningTexScale;
				Storm_LightningAnim		= TargetSnapshot.Storm_LightningAnim;
				Storm_LightningClouds	= TargetSnapshot.Storm_LightningClouds;
				Storm_LightningColor	= TargetSnapshot.Storm_LightningColor;
				Storm_LightningMasks	= TargetSnapshot.Storm_LightningMasks;
				Storm_AlbedoColor		= TargetSnapshot.Storm_AlbedoColor;
				bStorm = true;
				
				
				
				GetWorld()->GetTimerManager().SetTimer(
					SpawnTimerHandle,
					[this]()
					{
						auto Pawn = this->GetWorld()->GetFirstPlayerController()->GetPawn();
						FVector PawnPos = Pawn->GetActorLocation();
						FVector ForwardVector = Pawn->GetActorForwardVector();
						
						float Max = 5000.f;
						float U = FMath::FRandRange(0.6, 1.0f);
						float V = FMath::FRandRange(0.6, 1.0f);

						float R = Max;
						float Radius = R * FMath::Sqrt(U);
						float Theta = 2.0f * PI * V;

						float X = Radius * FMath::Cos(Theta);
						float Y = Radius * FMath::Sin(Theta);

						FVector Point = FVector(X, Y, 0.f);
			
						FTransform SpawnTransform = FTransform::Identity;
						FVector AddForward = ForwardVector*5500.f;
						SpawnTransform.SetLocation(Point + PawnPos + AddForward);
						if (!LightningEffect)
						{
							return;
						}

						UGameplayStatics::SpawnEmitterAtLocation(
							GetWorld(),
							LightningEffect,
							SpawnTransform.GetLocation(),
							FRotator::ZeroRotator,
							FVector(10.0f),
							true
						);
						
						UGameplayStatics::PlaySound2D(this,LightningSound);
					},
				15.0f,
				true
					);
				
				
			}
			UE_LOG(LogTemp, Warning,
				TEXT("CloudLerp Tick | Alpha: %.3f | Time: %.3f / %.3f | "),
				Alpha,
				ElapsedDuration,
				SafeDuration
			);
		}
	}
	
	if (VolumetricCloudComponentProxy)
	{
		if (IsValid(CloudDynamicMaterial))
		{
			CloudDynamicMaterial->SetScalarParameterValue(TEXT("StormClouds"),StormClouds);
			CloudDynamicMaterial->SetScalarParameterValue(TEXT("Storm_LightningTexScale"),Storm_LightningTexScale);
			CloudDynamicMaterial->SetVectorParameterValue(TEXT("Storm_LightningAnim"),Storm_LightningAnim);
			CloudDynamicMaterial->SetVectorParameterValue(TEXT("Storm_LightningClouds"),Storm_LightningClouds);
			CloudDynamicMaterial->SetVectorParameterValue(TEXT("Storm_LightningColor"),Storm_LightningColor);
			CloudDynamicMaterial->SetVectorParameterValue(TEXT("Storm_LightningMasks"),Storm_LightningMasks);
			CloudDynamicMaterial->SetVectorParameterValue(TEXT("Storm_AlbedoColor"),Storm_AlbedoColor);
		}
	}
	
	
	
}

void AWeatherVolumetricCloud::StateToTargetState(int TargetIdx, float InLerpTime)
{
	
	if (!CloudPreset.IsValidIndex(TargetIdx))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Target Index Is Not Valid"),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
		return;
	}
	
	FCloudSnapshot* Target = &CloudPreset[TargetIdx];
	
	CurrentSnapshot.StormClouds				= StormClouds;
	CurrentSnapshot.Storm_LightningTexScale = Storm_LightningTexScale;
	CurrentSnapshot.Storm_LightningAnim		= Storm_LightningAnim;
	CurrentSnapshot.Storm_LightningClouds	= Storm_LightningClouds;
	CurrentSnapshot.Storm_LightningColor	= Storm_LightningColor;
	CurrentSnapshot.Storm_LightningMasks	= Storm_LightningMasks;
	CurrentSnapshot.Storm_AlbedoColor		= Storm_AlbedoColor;
	
	TargetSnapshot.StormClouds				= Target->StormClouds;
	TargetSnapshot.Storm_LightningTexScale	= Target->Storm_LightningTexScale;
	TargetSnapshot.Storm_LightningAnim		= Target->Storm_LightningAnim;
	TargetSnapshot.Storm_LightningClouds	= Target->Storm_LightningClouds;
	TargetSnapshot.Storm_LightningColor		= Target->Storm_LightningColor;
	TargetSnapshot.Storm_LightningMasks		= Target->Storm_LightningMasks;
	TargetSnapshot.Storm_AlbedoColor		= Target->Storm_AlbedoColor;
	
	LerpDuration = InLerpTime;
	bStateToTargetState = true;
}

void AWeatherVolumetricCloud::CreateEffectRain(APlayerController* PlayerController)
{
	if (!IsValid(PlayerController))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Player Controller Is Not Valid"),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
		return;
	}
	
	APawn* Pawn = PlayerController->GetPawn();
	
	if (!IsValid(Pawn))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Pawn Is Not Valid"),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
		return;
	}
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = nullptr;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	ANiagaraActor* NiagaraActor = GetWorld()->SpawnActor<ANiagaraActor>(
		ANiagaraActor::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams
	);
	
	
	NiagaraActor->AttachToComponent(Pawn->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	
	if (NiagaraActor)
	{
		UNiagaraComponent* NiagaraComponent = NiagaraActor->GetNiagaraComponent();

		if (NiagaraComponent)
		{
			NiagaraComponent->SetAsset(RainEffect);
			NiagaraComponent->Activate(true);
		}
	}
	RainNiagaraActor = NiagaraActor;
}

void AWeatherVolumetricCloud::DeleteEffectRain()
{
	if (RainNiagaraActor) RainNiagaraActor->Destroy();	
}

