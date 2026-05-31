// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildSystem/WallComp.h"

#include "BuildSystem/RoofComp.h"

UWallComp::UWallComp()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(
		TEXT("/Game/BuildSystem/Wall")
	);

	if (MeshAsset.Succeeded())
	{
		SetStaticMesh(MeshAsset.Object);
		SetSimulatePhysics(false);
		SetMassOverrideInKg(NAME_None, 0.f, true);
	}
}

void UWallComp::DestroyRoof()
{
	if (Roof && Roof->IsValidLowLevel())
	{
		Roof->DestroyComponent();
	}
}
