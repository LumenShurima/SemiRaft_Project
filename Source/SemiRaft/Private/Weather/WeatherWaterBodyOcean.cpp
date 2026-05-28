#include "Weather/WeatherWaterBodyOcean.h"

#include "FrameTypes.h"
#include "WaterBodyComponent.h"
#include "WaterBodyOceanComponent.h"
#include "Curves/CurveFloat.h"
#include "Encounter/EncounterSubSystem.h"
#include "UObject/ConstructorHelpers.h"

AWeatherWaterBodyOcean::AWeatherWaterBodyOcean(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AWeatherWaterBodyOcean::PostLoad()
{
	Super::PostLoad();
	
	PrimaryActorTick.bCanEverTick = true;

	ApplyDefaultOceanSettings();
}

void AWeatherWaterBodyOcean::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (bWaveToTargetWave)
	{
		if (!CurrentWaterWaveGenerator)
		{
			UE_LOG(LogTemp, Error, TEXT("AWeatherWaterBodyOcean::Tick: CurrentWaterWaveGenerator is null."));

			bWaveToTargetWave = false;
			WaveLerpElapsedTime = 0.0f;
		}
		else
		{
			const float SafeDuration = FMath::Max(WaveLerpDuration, KINDA_SMALL_NUMBER);

			WaveLerpElapsedTime += DeltaSeconds;

			const float Alpha = FMath::Clamp(WaveLerpElapsedTime / SafeDuration, 0.0f, 1.0f);

			// NumWaves / Seed는 연속 보간 비추천.
			// 중간값으로 계속 바뀌면 파도 구성이 프레임마다 달라질 수 있음.
			CurrentWaterWaveGenerator->NumWaves = TargetSnapShot.NumWaves;
			CurrentWaterWaveGenerator->Seed = TargetSnapShot.Seed;

			CurrentWaterWaveGenerator->Randomness =
				FMath::Lerp(CurrentSnapShot.Randomness, TargetSnapShot.Randomness, Alpha);

			CurrentWaterWaveGenerator->MinWavelength =
				FMath::Lerp(CurrentSnapShot.MinWavelength, TargetSnapShot.MinWavelength, Alpha);

			CurrentWaterWaveGenerator->MaxWavelength =
				FMath::Lerp(CurrentSnapShot.MaxWavelength, TargetSnapShot.MaxWavelength, Alpha);

			CurrentWaterWaveGenerator->WavelengthFalloff =
				FMath::Lerp(CurrentSnapShot.WavelengthFalloff, TargetSnapShot.WavelengthFalloff, Alpha);

			CurrentWaterWaveGenerator->MinAmplitude =
				FMath::Lerp(CurrentSnapShot.MinAmplitude, TargetSnapShot.MinAmplitude, Alpha);

			CurrentWaterWaveGenerator->MaxAmplitude =
				FMath::Lerp(CurrentSnapShot.MaxAmplitude, TargetSnapShot.MaxAmplitude, Alpha);

			CurrentWaterWaveGenerator->AmplitudeFalloff =
				FMath::Lerp(CurrentSnapShot.AmplitudeFalloff, TargetSnapShot.AmplitudeFalloff, Alpha);

			CurrentWaterWaveGenerator->WindAngleDeg =
				FMath::Lerp(CurrentSnapShot.WindAngleDeg, TargetSnapShot.WindAngleDeg, Alpha);

			CurrentWaterWaveGenerator->DirectionAngularSpreadDeg =
				FMath::Lerp(CurrentSnapShot.DirectionAngularSpreadDeg, TargetSnapShot.DirectionAngularSpreadDeg, Alpha);

			CurrentWaterWaveGenerator->SmallWaveSteepness =
				FMath::Lerp(CurrentSnapShot.SmallWaveSteepness, TargetSnapShot.SmallWaveSteepness, Alpha);

			CurrentWaterWaveGenerator->LargeWaveSteepness =
				FMath::Lerp(CurrentSnapShot.LargeWaveSteepness, TargetSnapShot.LargeWaveSteepness, Alpha);

			CurrentWaterWaveGenerator->SteepnessFalloff =
				FMath::Lerp(CurrentSnapShot.SteepnessFalloff, TargetSnapShot.SteepnessFalloff, Alpha);

			if (WaterWaves)
			{
				if (UGerstnerWaterWaves* GerstnerWaves = Cast<UGerstnerWaterWaves>(WaterWaves->GetWaterWaves()))
				{
					GerstnerWaves->RecomputeWaves(false);
				}
			}

			if (Alpha >= 1.0f)
			{
				bWaveToTargetWave = false;
				WaveLerpElapsedTime = 0.0f;

				// 최종값 보정
				CurrentWaterWaveGenerator->NumWaves = TargetSnapShot.NumWaves;
				CurrentWaterWaveGenerator->Seed = TargetSnapShot.Seed;
				CurrentWaterWaveGenerator->Randomness = TargetSnapShot.Randomness;
				CurrentWaterWaveGenerator->MinWavelength = TargetSnapShot.MinWavelength;
				CurrentWaterWaveGenerator->MaxWavelength = TargetSnapShot.MaxWavelength;
				CurrentWaterWaveGenerator->WavelengthFalloff = TargetSnapShot.WavelengthFalloff;
				CurrentWaterWaveGenerator->MinAmplitude = TargetSnapShot.MinAmplitude;
				CurrentWaterWaveGenerator->MaxAmplitude = TargetSnapShot.MaxAmplitude;
				CurrentWaterWaveGenerator->AmplitudeFalloff = TargetSnapShot.AmplitudeFalloff;
				CurrentWaterWaveGenerator->WindAngleDeg = TargetSnapShot.WindAngleDeg;
				CurrentWaterWaveGenerator->DirectionAngularSpreadDeg = TargetSnapShot.DirectionAngularSpreadDeg;
				CurrentWaterWaveGenerator->SmallWaveSteepness = TargetSnapShot.SmallWaveSteepness;
				CurrentWaterWaveGenerator->LargeWaveSteepness = TargetSnapShot.LargeWaveSteepness;
				CurrentWaterWaveGenerator->SteepnessFalloff = TargetSnapShot.SteepnessFalloff;

				if (WaterWaves)
				{
					if (UGerstnerWaterWaves* GerstnerWaves = Cast<UGerstnerWaterWaves>(WaterWaves->GetWaterWaves()))
					{
						GerstnerWaves->RecomputeWaves(false);
					}
				}
			}
			UE_LOG(LogTemp, Warning,
				TEXT("WaveLerp Tick | Alpha: %.3f | Time: %.3f / %.3f | "
					 "NumWaves: %d -> %d | Seed: %d -> %d | "
					 "Randomness: %.3f | Wavelength: %.3f~%.3f | Amplitude: %.3f~%.3f | "
					 "WindAngle: %.3f | Spread: %.3f | Steepness: %.3f / %.3f / %.3f"),
				Alpha,
				WaveLerpElapsedTime,
				SafeDuration,

				CurrentSnapShot.NumWaves,
				TargetSnapShot.NumWaves,

				CurrentSnapShot.Seed,
				TargetSnapShot.Seed,

				CurrentWaterWaveGenerator->Randomness,
				CurrentWaterWaveGenerator->MinWavelength,
				CurrentWaterWaveGenerator->MaxWavelength,
				CurrentWaterWaveGenerator->MinAmplitude,
				CurrentWaterWaveGenerator->MaxAmplitude,
				CurrentWaterWaveGenerator->WindAngleDeg,
				CurrentWaterWaveGenerator->DirectionAngularSpreadDeg,
				CurrentWaterWaveGenerator->SmallWaveSteepness,
				CurrentWaterWaveGenerator->LargeWaveSteepness,
				CurrentWaterWaveGenerator->SteepnessFalloff
			);
		}
	}
}


UGerstnerWaterWaveGeneratorSimple* AWeatherWaterBodyOcean::GetWaterWaveGenerator(UWaterWavesBase* InWaterWaves)
{
	UGerstnerWaterWaves* Wave = Cast<UGerstnerWaterWaves>(InWaterWaves);
	
	if (!IsValid(Wave))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: WaterWavesAsset Is Not Valid."),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
		return nullptr;
	}
	
	UGerstnerWaterWaveGeneratorSimple* Gen = Cast<UGerstnerWaterWaveGeneratorSimple>(Wave->GerstnerWaveGenerator);
	if (!IsValid(Gen))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Generator Is Not Valid"),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
		return nullptr;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("%s::%s: Generator Is Valid"),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
	return Gen;
}

void AWeatherWaterBodyOcean::WaveToTargetWave(int TargetIdx, float InLerpTime)
{
	if (!CurrentWaterWaveGenerator)
	{
		CurrentWaterWaveGenerator = GetWaterWaveGenerator(WaterWaves);
	}
	const UGerstnerWaterWaveGeneratorSimple* Target = GetWaterWaveGenerator(WaterWavesArray[TargetIdx]->GetWaterWaves());
	
	
	if (!IsValid(Target))
	{
		UE_LOG(LogTemp, Error, TEXT("%s::%s: Target Generator Is Not Valid"),
			*GetClass()->GetName(), TEXT(__FUNCTION__));
		return;
	}
	
	CurrentSnapShot.NumWaves						= CurrentWaterWaveGenerator->NumWaves;
	CurrentSnapShot.Seed							= CurrentWaterWaveGenerator->Seed;
	CurrentSnapShot.Randomness						= CurrentWaterWaveGenerator->Randomness;
	CurrentSnapShot.MinWavelength					= CurrentWaterWaveGenerator->MinWavelength;
	CurrentSnapShot.MaxWavelength					= CurrentWaterWaveGenerator->MaxWavelength;
	CurrentSnapShot.WavelengthFalloff				= CurrentWaterWaveGenerator->WavelengthFalloff;
	CurrentSnapShot.MinAmplitude					= CurrentWaterWaveGenerator->MinAmplitude;
	CurrentSnapShot.MaxAmplitude					= CurrentWaterWaveGenerator->MaxAmplitude;
	CurrentSnapShot.AmplitudeFalloff				= CurrentWaterWaveGenerator->AmplitudeFalloff;
	CurrentSnapShot.WindAngleDeg					= CurrentWaterWaveGenerator->WindAngleDeg;
	CurrentSnapShot.DirectionAngularSpreadDeg		= CurrentWaterWaveGenerator->DirectionAngularSpreadDeg;
	CurrentSnapShot.SmallWaveSteepness				= CurrentWaterWaveGenerator->SmallWaveSteepness;
	CurrentSnapShot.LargeWaveSteepness				= CurrentWaterWaveGenerator->LargeWaveSteepness;
	CurrentSnapShot.SteepnessFalloff				= CurrentWaterWaveGenerator->SteepnessFalloff;
	
	TargetSnapShot.NumWaves							= Target->NumWaves;
	TargetSnapShot.Seed								= Target->Seed;
	TargetSnapShot.Randomness						= Target->Randomness;
	TargetSnapShot.MinWavelength					= Target->MinWavelength;
	TargetSnapShot.MaxWavelength					= Target->MaxWavelength;
	TargetSnapShot.WavelengthFalloff				= Target->WavelengthFalloff;
	TargetSnapShot.MinAmplitude						= Target->MinAmplitude;
	TargetSnapShot.MaxAmplitude						= Target->MaxAmplitude;
	TargetSnapShot.AmplitudeFalloff					= Target->AmplitudeFalloff;
	TargetSnapShot.WindAngleDeg						= Target->WindAngleDeg;
	TargetSnapShot.DirectionAngularSpreadDeg		= Target->DirectionAngularSpreadDeg;
	TargetSnapShot.SmallWaveSteepness				= Target->SmallWaveSteepness;
	TargetSnapShot.LargeWaveSteepness				= Target->LargeWaveSteepness;
	TargetSnapShot.SteepnessFalloff					= Target->SteepnessFalloff;
	
	WaveLerpDuration = InLerpTime;
	bWaveToTargetWave = true;
}

#if WITH_EDITOR
void AWeatherWaterBodyOcean::PostActorCreated()
{
	Super::PostActorCreated();

	ApplyDefaultOceanSettings();
}
#endif

void AWeatherWaterBodyOcean::ApplyDefaultOceanSettings()
{
	UWaterBodyOceanComponent* OceanComp = Cast<UWaterBodyOceanComponent>(GetWaterBodyComponent());
	if (!OceanComp)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("AWeatherWaterBodyOcean::ApplyDefaultOceanSettings: WaterBodyComponent is not UWaterBodyOceanComponent.")
		);
		return;
	}

#if WITH_EDITOR
	Modify();
	OceanComp->Modify();
#endif

	// ============================================================
	// Actor HLOD Settings
	// ============================================================

	// 원본 Ocean 기본값:
	// bEnableAutoLODGeneration = True
	bEnableAutoLODGeneration = true;

	// ============================================================
	// Water Surface Material
	// ============================================================

	UMaterialInterface* WaterMaterial = LoadObject<UMaterialInterface>(
		nullptr,
		TEXT("/Water/Materials/WaterSurface/Water_Material_Ocean.Water_Material_Ocean")
	);

	if (WaterMaterial)
	{
		OceanComp->SetWaterMaterial(WaterMaterial);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("AWeatherWaterBodyOcean: Failed to load Water_Material_Ocean.")
		);
	}

	// ============================================================
	// Underwater Post Process Material
	// ============================================================

	UMaterialInterface* UnderwaterPostProcessMaterial = LoadObject<UMaterialInterface>(
		nullptr,
		TEXT("/Water/Materials/PostProcessing/M_UnderWater_PostProcess_Volume.M_UnderWater_PostProcess_Volume")
	);

	if (UnderwaterPostProcessMaterial)
	{
		OceanComp->SetUnderwaterPostProcessMaterial(UnderwaterPostProcessMaterial);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("AWeatherWaterBodyOcean: Failed to load M_UnderWater_PostProcess_Volume.")
		);
	}

	// ============================================================
	// Water Info Material
	// ============================================================

	UMaterialInterface* WaterInfoMaterial = LoadObject<UMaterialInterface>(
		nullptr,
		TEXT("/Water/Materials/WaterInfo/DrawWaterInfo.DrawWaterInfo")
	);

	if (WaterInfoMaterial)
	{
		// UE 버전에 따라 private/protected일 수 있음.
		// 접근 에러가 나면 UWaterBodyComponent 쪽에서 SetWaterInfoMaterial 계열 setter를 찾아야 함.
		OceanComp->WaterInfoMaterial = WaterInfoMaterial;
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("AWeatherWaterBodyOcean: Failed to load DrawWaterInfo.")
		);
	}

	// ============================================================
	// Water Static Mesh LOD Material
	// ============================================================

	UMaterialInterface* WaterStaticMeshMaterial = LoadObject<UMaterialInterface>(
		nullptr,
		TEXT("/Water/Materials/WaterSurface/LODs/Water_Material_Ocean_LOD.Water_Material_Ocean_LOD")
	);

	if (WaterStaticMeshMaterial)
	{
		// UE 버전에 따라 private/protected일 수 있음.
		// 접근 에러가 나면 UWaterBodyComponent / UWaterBodyOceanComponent에서 setter 검색.
		OceanComp->WaterStaticMeshMaterial = WaterStaticMeshMaterial;
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("AWeatherWaterBodyOcean: Failed to load Water_Material_Ocean_LOD.")
		);
	}

	// ============================================================
	// Water HLOD Material
	// ============================================================

	UMaterialInterface* WaterHLODMaterial = LoadObject<UMaterialInterface>(
		nullptr,
		TEXT("/Water/Materials/HLOD/HLODWater.HLODWater")
	);

	if (WaterHLODMaterial)
	{
		// UE 버전에 따라 private/protected일 수 있음.
		// 접근 에러가 나면 SetWaterHLODMaterial 같은 setter가 있는지 검색.
		OceanComp->WaterHLODMaterial = WaterHLODMaterial;
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("AWeatherWaterBodyOcean: Failed to load HLODWater.")
		);
	}

	// ============================================================
	// Visibility
	// ============================================================

	OceanComp->SetVisibility(true);
	OceanComp->SetHiddenInGame(false);

	// ============================================================
	// Underwater Post Process Basic Settings
	// ============================================================

	OceanComp->UnderwaterPostProcessSettings.bEnabled = true;
	OceanComp->UnderwaterPostProcessSettings.Priority = 0.0f;
	OceanComp->UnderwaterPostProcessSettings.BlendRadius = 100.0f;
	OceanComp->UnderwaterPostProcessSettings.BlendWeight = 1.0f;

	// ============================================================
	// Curve Settings
	// ============================================================

	OceanComp->CurveSettings.bUseCurveChannel = true;
	OceanComp->CurveSettings.ChannelEdgeOffset = -1000.0f;
	OceanComp->CurveSettings.ChannelDepth = 2000.0f;
	OceanComp->CurveSettings.CurveRampWidth = 8000.0f;

	UCurveFloat* ElevationCurve = LoadObject<UCurveFloat>(
		nullptr,
		TEXT("/Water/Curves/FloatCurve.FloatCurve")
	);

	if (ElevationCurve)
	{
		OceanComp->CurveSettings.ElevationCurveAsset = ElevationCurve;
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("AWeatherWaterBodyOcean: Failed to load /Water/Curves/FloatCurve.FloatCurve.")
		);
	}

	// ============================================================
	// Landscape / Heightmap Settings
	// ============================================================

	OceanComp->bAffectsLandscape = true;

	// 원본 덤프:
	// WaterHeightmapSettings.BlendMode = AlphaBlend
	// WaterHeightmapSettings.FalloffSettings.FalloffMode = Angle
	//
	// 아래 enum 이름은 UE 버전에 따라 다를 수 있음.
	// 컴파일 에러 나면 FWaterBodyHeightmapSettings / FWaterFalloffSettings 정의에서 타입명 확인.
	//
	// OceanComp->WaterHeightmapSettings.BlendMode = EWaterBrushBlendType::AlphaBlend;
	// OceanComp->WaterHeightmapSettings.FalloffSettings.FalloffMode = EWaterBrushFalloffMode::Angle;

	OceanComp->WaterHeightmapSettings.FalloffSettings.FalloffAngle = 45.0f;
	OceanComp->WaterHeightmapSettings.FalloffSettings.FalloffWidth = 1024.0f;
	OceanComp->WaterHeightmapSettings.FalloffSettings.EdgeOffset = 1000.0f;
	OceanComp->WaterHeightmapSettings.FalloffSettings.ZOffset = 32.0f;

	// ============================================================
	// Heightmap Effects - Blurring
	// ============================================================

	OceanComp->WaterHeightmapSettings.Effects.Blurring.bBlurShape = true;
	OceanComp->WaterHeightmapSettings.Effects.Blurring.Radius = 2;

	// ============================================================
	// Heightmap Effects - Curl Noise
	// ============================================================

	OceanComp->WaterHeightmapSettings.Effects.CurlNoise.Curl1Amount = 0.0f;
	OceanComp->WaterHeightmapSettings.Effects.CurlNoise.Curl2Amount = 0.0f;
	OceanComp->WaterHeightmapSettings.Effects.CurlNoise.Curl1Tiling = 16.0f;
	OceanComp->WaterHeightmapSettings.Effects.CurlNoise.Curl2Tiling = 3.0f;

	// ============================================================
	// Heightmap Effects - Displacement
	// ============================================================

	OceanComp->WaterHeightmapSettings.Effects.Displacement.DisplacementHeight = 0.0f;
	OceanComp->WaterHeightmapSettings.Effects.Displacement.DisplacementTiling = 0.0f;
	OceanComp->WaterHeightmapSettings.Effects.Displacement.Texture = nullptr;
	OceanComp->WaterHeightmapSettings.Effects.Displacement.Midpoint = -128.0f;
	OceanComp->WaterHeightmapSettings.Effects.Displacement.Channel = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
	OceanComp->WaterHeightmapSettings.Effects.Displacement.WeightmapInfluence = 0.0f;

	// ============================================================
	// Heightmap Effects - Smooth Blending
	// ============================================================

	OceanComp->WaterHeightmapSettings.Effects.SmoothBlending.InnerSmoothDistance = 0.01f;
	OceanComp->WaterHeightmapSettings.Effects.SmoothBlending.OuterSmoothDistance = 0.01f;

	// ============================================================
	// Heightmap Effects - Terracing
	// ============================================================

	OceanComp->WaterHeightmapSettings.Effects.Terracing.TerraceAlpha = 0.0f;
	OceanComp->WaterHeightmapSettings.Effects.Terracing.TerraceSpacing = 256.0f;
	OceanComp->WaterHeightmapSettings.Effects.Terracing.TerraceSmoothness = 0.0f;
	OceanComp->WaterHeightmapSettings.Effects.Terracing.MaskLength = 0.0f;
	OceanComp->WaterHeightmapSettings.Effects.Terracing.MaskStartOffset = 0.0f;

#if WITH_EDITOR
	OceanComp->Modify();
#endif
}