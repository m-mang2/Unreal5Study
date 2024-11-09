// Fill out your copyright notice in the Description page of Project Settings.


#include "Cropout/Island/USIslandGenerator.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "ProceduralMeshComponent.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "GeometryScript/MeshVoxelFunctions.h"
#include "GeometryScript/MeshDeformFunctions.h"
#include "GeometryScript/MeshSubdivideFunctions.h"
#include "GeometryScript/MeshNormalsFunctions.h"
#include "GeometryScript/MeshBooleanFunctions.h"
#include "GeometryScript/GeometryScriptSelectionTypes.h"
#include "GeometryScript/MeshUVFunctions.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "NavigationSystem.h"
#include "../../GameMode/USCropoutGameMode.h"

AUSIslandGenerator::AUSIslandGenerator()
{
}

void AUSIslandGenerator::BeginPlay()
{
	Super::BeginPlay();
	
	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
	if (GameMode)
	{
		AUSCropoutGameMode* CropoutGameMode = Cast<AUSCropoutGameMode>(GameMode);
		if (CropoutGameMode )
		{
			OnTaskComplete.AddDynamic(CropoutGameMode, &AUSCropoutGameMode::IslandGencomplete);
		}
	}
	

	DynamicMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DynamicMeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

    SpawnCone();
	SpawnMarker();
	AppendBox();
	MeshSlidify();
	SmoothMesh();
	Tesselation();

	MeshPlaneCut();
	ProjectUvs();
	ReleaseCompute();
	SetIslandColor();

	OnTaskComplete.Broadcast();
}

void AUSIslandGenerator::SpawnCone()
{
    for (int32 i = 0; i < Islands; i++)
    {
        // ��ġ ����
        FVector RandSpawnPoint = UKismetMathLibrary::RandomUnitVector() * (MaxSpawnDistance / 2.0f);
        float Radius = FMath::RandRange(IslandSizeX, IslandSizeY);
		RandSpawnPoint.Z = 0;
		SpawnPoints.Add(RandSpawnPoint);

        // ��ȯ ����
        FVector Location(RandSpawnPoint.X, RandSpawnPoint.Y, -800.0f); // Z ���� ����
        FRotator Rotation(0.0f, 0.0f, 0.0f); // ȸ���� ����
        FVector Scale(1.0f, 1.0f, 1.0f); // ������

        FTransform Transform(Rotation, Location, Scale);

        FGeometryScriptPrimitiveOptions PrimitiveOptions;
		UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendCone(
			DynamicMeshComponent->GetDynamicMesh(),
			PrimitiveOptions,
			Transform,
			Radius,
			Radius / 4.0f,
			1300,
			32,
			1,
			true
		);
    }
}

void AUSIslandGenerator::SpawnMarker()
{
	if (SpawnMarkerClass == nullptr)
		return;

	for (auto SpawnPoint : SpawnPoints)
	{
		FVector SpawnLocation = SpawnPoint;
		FRotator SpawnRotation = FRotator::ZeroRotator;
		FActorSpawnParameters SpawnParams;
		GetWorld()->SpawnActor<AActor>(SpawnMarkerClass, SpawnLocation, SpawnRotation, SpawnParams);
	}	
}

void AUSIslandGenerator::AppendBox()
{
	FVector Location(0, 0, -800.0f); // Z ���� ����
	FRotator Rotation(0.0f, 0.0f, 0.0f); // ȸ���� ����
	FVector Scale(1.0f, 1.0f, 1.0f); // ������
	FTransform Transform(Rotation, Location, Scale);
	
	FGeometryScriptPrimitiveOptions PrimitiveOptions;
	UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendBox(
		DynamicMeshComponent->GetDynamicMesh(),
		PrimitiveOptions,
		Transform,
		MaxSpawnDistance + 10000.0f,
		MaxSpawnDistance + 10000.0f,
		400.0f
	);
}

void AUSIslandGenerator::MeshSlidify()
{
	FGeometryScriptSolidifyOptions Options;
	Options.WindingThreshold = 0.5f;
	Options.SurfaceSearchSteps = 64;
	Options.ShellThickness = 1.0f;
	Options.GridParameters.GridCellSize = 0.25f;

	UGeometryScriptLibrary_MeshVoxelFunctions::ApplyMeshSolidify(
		DynamicMeshComponent->GetDynamicMesh(),
		Options
	);

	UGeometryScriptLibrary_MeshNormalsFunctions::SetPerVertexNormals(
		DynamicMeshComponent->GetDynamicMesh()
	);
}

void AUSIslandGenerator::SmoothMesh()
{
	FGeometryScriptMeshSelection Selection;
	FGeometryScriptIterativeMeshSmoothingOptions Options;
	Options.NumIterations = 6;
	Options.Alpha = 0.2;

	UGeometryScriptLibrary_MeshDeformFunctions::ApplyIterativeSmoothingToMesh(
		DynamicMeshComponent->GetDynamicMesh(),
		Selection,
		Options
	);
}

void AUSIslandGenerator::Tesselation()
{
	FGeometryScriptPNTessellateOptions Options;
	UGeometryScriptLibrary_MeshSubdivideFunctions::ApplyPNTessellation(
		DynamicMeshComponent->GetDynamicMesh(),
		Options,
		2
	);
}

void AUSIslandGenerator::MeshPlaneCut()
{
	FVector Location(0.0f, 0.0f, -390.0);
	FRotator Rotation(180.0, 0.0f, 0.0f);
	FVector Scale(1.0f, 1.0f, 1.0f);
	FTransform Transform(Rotation, Location, Scale);

	FGeometryScriptMeshPlaneCutOptions Options;
	Options.bFillHoles = false;
	Options.bFillSpans = false;
	Options.bFlipCutSide = false;

	UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshPlaneCut(
		DynamicMeshComponent->GetDynamicMesh(),
		Transform,
		Options
	);
}

void AUSIslandGenerator::ProjectUvs()
{
	FVector Location(0.0f, 0.0f, 0.0f);
	FRotator Rotation(0.0f, 0.0f, 0.0f);
	FVector Scale(1.0f, 1.0f, 1.0f);
	FTransform Transform(Rotation, Location, Scale);

	FGeometryScriptMeshPlaneCutOptions Options;

	UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshPlaneCut(
		DynamicMeshComponent->GetDynamicMesh(),
		Transform,
		Options
	);

	FVector UVScale(100.0f, 100.0f, 100.0f);
	FTransform UVTransform(Rotation, Location, UVScale);
	FGeometryScriptMeshSelection Selection;
	UGeometryScriptLibrary_MeshUVFunctions::SetMeshUVsFromPlanarProjection(
		DynamicMeshComponent->GetDynamicMesh(),
		0,
		UVTransform,
		Selection
	);
}

void AUSIslandGenerator::ReleaseCompute()
{
	ReleaseAllComputeMeshes();

	FVector Location(0.0f, 0.0f, 0.05f);
	AddActorWorldOffset(Location);

	FNavigationSystem::UpdateComponentData(*DynamicMeshComponent);
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		NavSys->Build();
	}

}

void AUSIslandGenerator::SetIslandColor()
{
	FString MaterialCollectionPath = TEXT("/Game/Study/Cropout/Data/MPC_Landscape.MPC_Landscape");

	UMaterialParameterCollection* MaterialCollection = Cast<UMaterialParameterCollection>(
		StaticLoadObject(UMaterialParameterCollection::StaticClass(), nullptr, *MaterialCollectionPath)
	);

	if (MaterialCollection)
	{
		FRandomStream Stream;
		float RandomHueShift = UKismetMathLibrary::RandomFloatInRangeFromStream(Stream, 0.0f, 90.0f) + 102.999725;
		FLinearColor GrassColour = UKismetMaterialLibrary::GetVectorParameterValue(GetWorld(), MaterialCollection, TEXT("GrassColour"));

		float H, S, V, A;
		UKismetMathLibrary::RGBToHSV(GrassColour, H, S, V, A);
		FLinearColor NewGrassColour = UKismetMathLibrary::HSVToRGB(RandomHueShift, S, V, A);

		UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(), MaterialCollection, TEXT("GrassColour"), NewGrassColour);
	}

}
