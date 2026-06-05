// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildSystem/FloorComp.h"

#include "BuildSystem/RoofComp.h"
#include "BuildSystem/WallComp.h"

UFloorComp::UFloorComp()
{

}

void UFloorComp::AddWalls(UWallComp* wall)
{
	Walls.Add(wall);
}

void UFloorComp::AllWallsDestroy()
{
	for (int i = 0; i < Walls.Num(); i++)
	{
		if (Walls[i] && Walls[i]->IsValidLowLevel())
		{
			if (Walls[i]->Roof && Walls[i]->Roof->IsValidLowLevel())
			{
				Walls[i]->Roof->DestroyComponent();
			}
			Walls[i]->DestroyComponent();
		}
	}
}

void UFloorComp::RemoveWall(UWallComp* wall)
{
	if (Walls.Contains(wall))
	{
		Walls.Remove(wall);
	}
}
