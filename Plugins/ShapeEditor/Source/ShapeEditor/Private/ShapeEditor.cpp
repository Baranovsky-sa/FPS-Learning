// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShapeEditor.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "Engine/World.h"
#include "Editor.h"
#include "ToolMenuSection.h"
#include "Components/DynamicMeshComponent.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"

#define LOCTEXT_NAMESPACE "FShapeEditorModule"

void FShapeEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FShapeEditorModule::RegisterMenus));
}

void FShapeEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FShapeEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnedScoped(this);

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
	FToolMenuSection& Section = Menu->AddSection("ShapeEditor", FText::FromString("Shape Tools"));
	Section.AddMenuEntry(
		"GenerateCube",
		FText::FromString("Generate Dynamic Cube"),
		FText::FromString("Creates a cube using DynamicMeshComponent"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FShapeEditorModule::GenerateCube))
		);
}

void FShapeEditorModule::GenerateCube()
{
	UWorld* World = GEditor->GetEditorWorldContext().World();
	//if (World) return;

	AActor* ShapeActor = World->SpawnActor<AActor>();
	ShapeActor->SetActorLabel(TEXT("DynamicCube"));

	UDynamicMeshComponent* MeshComp = NewObject<UDynamicMeshComponent>(ShapeActor);
	MeshComp->RegisterComponent();
	MeshComp->SetMobility(EComponentMobility::Movable);
	MeshComp->SetWorldLocation(FVector(0, 0, 100));

	ShapeActor->AddInstanceComponent(MeshComp);
	ShapeActor->SetRootComponent(MeshComp);

	UDynamicMesh* DynamicMesh = MeshComp->GetDynamicMesh();

	FGeometryScriptPrimitiveOptions PrimitiveOptions;
	PrimitiveOptions.PolygroupMode = EGeometryScriptPrimitivePolygroupMode::PerFace;
	PrimitiveOptions.bFlipOrientation = false;
	PrimitiveOptions.UVMode = EGeometryScriptPrimitiveUVMode::Uniform;

	FTransform BoxTransform = FTransform(FRotator::ZeroRotator, FVector(920, 730, 200), FVector(1));

	UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendBox(
		DynamicMesh,
		PrimitiveOptions,
		BoxTransform,
		100.0f, 100.0f, 100.0f,  // Dimensions
		1, 1, 1,                 // Steps
		EGeometryScriptPrimitiveOriginMode::Center,
		nullptr                  // Debug
	);

	MeshComp->NotifyMeshUpdated();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FShapeEditorModule, ShapeEditor)