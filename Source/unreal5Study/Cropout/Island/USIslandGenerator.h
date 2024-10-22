// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DynamicMeshActor.h"
#include "USIslandGenerator.generated.h"

/**
 * 
 */
UCLASS()
class UNREAL5STUDY_API AUSIslandGenerator : public ADynamicMeshActor
{
	GENERATED_BODY()
	
public:
	AUSIslandGenerator();
	
	virtual void BeginPlay() override;

protected:
	void SpawnCone();
	void AppendBox();
	void MeshSlidify();
	void SmoothMesh();
	void Tesselation();

	void MeshPlaneCut();
	void ProjectUvs();
	void ReleaseCompute();
	void SetIslandColor();
private:
	int32 Islands = 20; // ������� ���� ����
	float MaxSpawnDistance = 9776.351562;
	float IslandSizeX = 800.0;
	float IslandSizeY = 5000.0;
};
