// Copyright StudioThursday, Inc. All Rights Reserved.


#include "ChartEditorImportHookSubsystem.h"
#include "UObject/Class.h"
#include "Subsystems/Subsystem.h"
#include "EditorSubsystem.h"
#include "Subsystems/ImportSubsystem.h"
#include "ChartEditorFunctionLibraly.h"
#include "ChartDataAsset.h"
#include "FileHelpers.h"

void UChartEditorImportHookSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    // ImportSubsystem‚ª–¢‰Šú‰»‚È‚ç‚Îæ‚É‰Šú‰»‚·‚é.
    Collection.InitializeDependency(UImportSubsystem::StaticClass());

    UImportSubsystem* ImportSubsystem = GEditor->GetEditorSubsystem<UImportSubsystem>();

    // “o˜^
    ImportSubsystem->OnAssetPreImport.AddUObject(this, &UChartEditorImportHookSubsystem::OnAssetPreImportFunc);
    ImportSubsystem->OnAssetPostImport.AddUObject(this, &UChartEditorImportHookSubsystem::OnAssetPostImportFunc);
    ImportSubsystem->OnAssetReimport.AddUObject(this, &UChartEditorImportHookSubsystem::FOnAssetReimportFunc);
    ImportSubsystem->OnAssetPostLODImport.AddUObject(this, &UChartEditorImportHookSubsystem::FOnAssetPostLODImportFunc);
}

void UChartEditorImportHookSubsystem::Deinitialize()
{
}

void UChartEditorImportHookSubsystem::OnAssetPreImportFunc(UFactory* InFactory, UClass* InClass, UObject* InParent, const FName& Name, const TCHAR* Type)
{
}

void UChartEditorImportHookSubsystem::OnAssetPostImportFunc(UFactory* InFactory, UObject* InCreatedObject)
{
}

void UChartEditorImportHookSubsystem::FOnAssetReimportFunc(UObject* InCreatedObject)
{
   UDataTable* DataTable = Cast<UDataTable>(InCreatedObject);
   if (!DataTable) { return; }
   if (!UChartEditorFunctionLibraly::CheckIsSourceDataTable(*DataTable)) { return; }
   if (UDataTable* GameplayTagDataTable = UChartEditorFunctionLibraly::CreateGameplayTagDataTable(*DataTable))
   {
	   UChartEditorFunctionLibraly::AddGameplayTagDataTableToSettings(*GameplayTagDataTable);
	   if (UChartDataAsset* ChartDataAsset = UChartEditorFunctionLibraly::CreateChartDataAsset(*DataTable, *GameplayTagDataTable))
	   {
		   ChartDataAsset->Build();
	   }
   }
}

void UChartEditorImportHookSubsystem::FOnAssetPostLODImportFunc(UObject* InObject, int32 inLODIndex)
{
}
