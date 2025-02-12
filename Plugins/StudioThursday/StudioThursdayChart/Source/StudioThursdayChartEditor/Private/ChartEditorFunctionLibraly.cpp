// Copyright StudioThursday, Inc. All Rights Reserved.


#include "ChartEditorFunctionLibraly.h"
#include "AssetToolsModule.h"
#if ENGINE_MAJOR_VERSION > 5 || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 5)
#include "AssetRegistry/AssetRegistryModule.h"
#else
#include "AssetRegistryModule.h"
#endif
#include "Engine/DataTable.h"
#include "Engine/UserDefinedEnum.h"
#include "UObject/Class.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "GameplayTagsManager.h"
#include "GameplayTagsSettings.h"
#include "FileHelpers.h"
#include "SourceControlHelpers.h"
#include "Internationalization/Regex.h"
#include "ChartDataAsset.h"
#include "StudioThursdayChartEditorSettings.h"

const FName UChartEditorFunctionLibraly::DUMMY = FName(TEXT("{DUMMY}"));

void UChartEditorFunctionLibraly::AddGameplayTagDataTableToSettings(const UDataTable& DataTable)
{
	UGameplayTagsSettings* Settings = GetMutableDefault<UGameplayTagsSettings>();
	FSoftObjectPath Path(&DataTable);
	if (!Settings->GameplayTagTableList.Contains(Path))
	{
		if (CheckOutFileIfNeed(Settings->GetDefaultConfigFilename(), false))
		{
			// HACK: If the array is empty, a line with "= __ClearArray__" will be duplicated in the ini file each time you SaveConfig.
			if (Settings->CategoryRemapping.Num() == 0) { Settings->CategoryRemapping.Add(FGameplayTagCategoryRemap()); }
			if (Settings->GameplayTagRedirects.Num() == 0) { Settings->GameplayTagRedirects.Add(FGameplayTagRedirect()); }
			if (Settings->CommonlyReplicatedTags.Num() == 0)
			{
				// HACK: If the entry is an empty string, the engine will not start the next time.
				Settings->GameplayTagList.AddUnique(FGameplayTagTableRow(DUMMY));
				Settings->CommonlyReplicatedTags.Add(DUMMY); 
			}
			if (Settings->RestrictedConfigFiles.Num() == 0) { Settings->RestrictedConfigFiles.Add(FRestrictedConfigInfo()); }

			Settings->GameplayTagTableList.AddUnique(Path);
			Settings->SaveConfig(CPF_Config, *Settings->GetDefaultConfigFilename());
		}
	}
	UGameplayTagsManager::Get().EditorRefreshGameplayTagTree();
}

UDataTable* UChartEditorFunctionLibraly::CreateGameplayTagDataTable(const UDataTable& DataTable)
{
	const UStudioThursdayChartEditorSettings* StudioThursdayChartEditorSettings = GetDefault<UStudioThursdayChartEditorSettings>();
	FString BaseAssetName = DataTable.GetName().Mid(StudioThursdayChartEditorSettings->SourceDataTablePrefix.Len()).LeftChop(StudioThursdayChartEditorSettings->SourceDataTablePostfix.Len());
	FString GameplayTagDataTableAssetName = FString::Printf(TEXT("%s%s%s"), *StudioThursdayChartEditorSettings->GameplayTagDataAssetPrefix, *BaseAssetName, *StudioThursdayChartEditorSettings->GameplayTagDataAssetPostfix);
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> AssetDatas;
	AssetRegistry.GetAssetsByPath(FName(StudioThursdayChartEditorSettings->GameplayTagDataTableDirectory.Path), AssetDatas);
	UObject* Obj = nullptr;
	for (int32 i = 0; i < AssetDatas.Num(); ++i)
	{
		if (!AssetDatas[i].AssetName.IsEqual(FName(GameplayTagDataTableAssetName))) { continue; }
		Obj = AssetDatas[i].GetAsset();
		break;
	}
	if (!Obj)
	{
		FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
		Obj = AssetToolsModule.Get().CreateAsset(GameplayTagDataTableAssetName, StudioThursdayChartEditorSettings->GameplayTagDataTableDirectory.Path, UDataTable::StaticClass(), nullptr, TEXT(""));
	}
	if (!Obj) { return nullptr; }

	UDataTable* GameplayTagDataTable = Cast<UDataTable>(Obj);
	if (!GameplayTagDataTable) { return nullptr; }

	GameplayTagDataTable->EmptyTable();
	GameplayTagDataTable->RowStruct = FGameplayTagTableRow::StaticStruct();
	TArray<FName> RowNames = DataTable.GetRowNames();
	for (int32 i = 0; i < RowNames.Num(); ++i)
	{
		FGameplayTagTableRow GameplayTagRow;
		GameplayTagRow.Tag = RowNames[i];
		GameplayTagDataTable->AddRow(RowNames[i], GameplayTagRow);
	}
	GameplayTagDataTable->MarkPackageDirty();
	return GameplayTagDataTable;
}

UChartDataAsset* UChartEditorFunctionLibraly::CreateChartDataAsset(UDataTable& DataTable, const UDataTable& GameplayTagDataTable)
{
	const UStudioThursdayChartEditorSettings* StudioThursdayChartEditorSettings = GetDefault<UStudioThursdayChartEditorSettings>();
	FString BaseAssetName = DataTable.GetName().Mid(StudioThursdayChartEditorSettings->SourceDataTablePrefix.Len()).LeftChop(StudioThursdayChartEditorSettings->SourceDataTablePostfix.Len());
	FString DataAssetName = FString::Printf(TEXT("%s%s%s"), *StudioThursdayChartEditorSettings->ChartDataAssetPrefix, *BaseAssetName, *StudioThursdayChartEditorSettings->ChartDataAssetPostfix);
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> AssetDatas;
	AssetRegistry.GetAssetsByPath(FName(StudioThursdayChartEditorSettings->ChartDataAssetDirectory.Path), AssetDatas);
	UObject* Obj = nullptr;
	for (int32 i = 0; i < AssetDatas.Num(); ++i) {
		if (!AssetDatas[i].AssetName.IsEqual(FName(DataAssetName))) { continue; }
		Obj = AssetDatas[i].GetAsset();
		break;
	}
	if (!Obj) {
		FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
		Obj = AssetToolsModule.Get().CreateAsset(DataAssetName, StudioThursdayChartEditorSettings->ChartDataAssetDirectory.Path, UChartDataAsset::StaticClass(), nullptr, TEXT(""));
	}
	if (!Obj) { return nullptr; }

	UChartDataAsset* ChartDataAsset = Cast<UChartDataAsset>(Obj);
	if (!ChartDataAsset) { return nullptr; }

	ChartDataAsset->DataTable = &DataTable;
	ChartDataAsset->GameplayTagDataTable = &GameplayTagDataTable;
	ChartDataAsset->MarkPackageDirty();
	return ChartDataAsset;
}

bool UChartEditorFunctionLibraly::CheckIsSourceDataTable(const UDataTable& DataTable)
{
	const UStudioThursdayChartEditorSettings* StudioThursdayChartEditorSettings = GetDefault<UStudioThursdayChartEditorSettings>();
	const FRegexPattern RegexPattern = FRegexPattern(FString::Printf(TEXT("%s.*%s"), *StudioThursdayChartEditorSettings->SourceDataTablePrefix, *StudioThursdayChartEditorSettings->SourceDataTablePostfix));
	FRegexMatcher RegexMatcher = FRegexMatcher(RegexPattern, DataTable.GetName());
	return RegexMatcher.FindNext();
}

bool UChartEditorFunctionLibraly::CheckOutFileIfNeed(const FString& InFile, bool bSilent)
{
	if (USourceControlHelpers::IsEnabled())
	{
		return USourceControlHelpers::CheckOutFile(InFile, bSilent);
	}
	return true;
}
