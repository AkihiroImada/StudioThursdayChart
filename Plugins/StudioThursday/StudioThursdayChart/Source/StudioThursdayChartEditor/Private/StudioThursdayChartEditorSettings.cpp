// Copyright StudioThursday, Inc. All Rights Reserved.


#include "StudioThursdayChartEditorSettings.h"

#define LOCTEXT_NAMESPACE "StudioThursdayChartEditorSettings"

UStudioThursdayChartEditorSettings::UStudioThursdayChartEditorSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	GameplayTagDataTableDirectory.Path = FString(TEXT("/Game/"));
	ChartDataAssetDirectory.Path = FString(TEXT("/Game/"));
	SourceDataTablePrefix = FString(TEXT("DT_"));
	SourceDataTablePostfix = FString(TEXT(""));
	ChartDataAssetPrefix = FString("DA_");
	ChartDataAssetPostfix = FString("");
	GameplayTagDataAssetPrefix = FString("DT_");
	GameplayTagDataAssetPostfix = FString("Tag");
}

FName UStudioThursdayChartEditorSettings::GetContainerName() const
{
	return Super::GetContainerName();
}

FName UStudioThursdayChartEditorSettings::GetCategoryName() const
{
	return "Plugins";
}

FName UStudioThursdayChartEditorSettings::GetSectionName() const
{
	return Super::GetSectionName();
}

FText UStudioThursdayChartEditorSettings::GetSectionText() const
{
	return LOCTEXT("UStudioThursdayChartEditorSettings::GetSectionText", "StudioThursdayChart Editor");
}

FText UStudioThursdayChartEditorSettings::GetSectionDescription() const
{
	return LOCTEXT("UStudioThursdayChartEditorSettings::GetSectionDescription", "StudioThursdayChart Editor Settings");
}

#undef LOCTEXT_NAMESPACE
