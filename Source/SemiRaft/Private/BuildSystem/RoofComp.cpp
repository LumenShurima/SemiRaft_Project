// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildSystem/RoofComp.h"

URoofComp::URoofComp()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(
		TEXT("/Game/BuildSystem/Roof")
	);

	if (MeshAsset.Succeeded())
	{
		SetStaticMesh(MeshAsset.Object);
		SetSimulatePhysics(false);
		SetMassOverrideInKg(NAME_None, 0.f, true);
	}
}
