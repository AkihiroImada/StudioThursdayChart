// Copyright StudioThursday, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "StudioThursdayChartEditorSettings.generated.h"

/**
 * 
 */
UCLASS(config = Editor, defaultconfig)
class UStudioThursdayChartEditorSettings : public UDeveloperSettings 
{
	GENERATED_BODY()
public:
	UStudioThursdayChartEditorSettings(const FObjectInitializer& ObjectInitializer);
	FName GetContainerName() const override;
	FName GetCategoryName() const override;
	FName GetSectionName() const override;
	FText GetSectionText() const override;
	FText GetSectionDescription() const override;

	UPROPERTY(EditAnyWhere, Config, meta = (ContentDir))
	FDirectoryPath GameplayTagDataTableDirectory;
	UPROPERTY(EditAnyWhere, Config, meta = (ContentDir))
	FDirectoryPath ChartDataAssetDirectory;
	UPROPERTY(EditAnyWhere, Config)
	FString SourceDataTablePrefix;
	UPROPERTY(EditAnyWhere, Config)
	FString SourceDataTablePostfix;
	UPROPERTY(EditAnyWhere, Config)
	FString ChartDataAssetPrefix;
	UPROPERTY(EditAnyWhere, Config)
	FString ChartDataAssetPostfix;
	UPROPERTY(EditAnyWhere, Config)
	FString GameplayTagDataAssetPrefix;
	UPROPERTY(EditAnyWhere, Config)
	FString GameplayTagDataAssetPostfix;
};
