// Copyright StudioThursday, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChartEditorFunctionLibraly.generated.h"

class UDataTable;
class UChartDataAsset;

/**
 * 
 */
UCLASS()
class STUDIOTHURSDAYCHARTEDITOR_API UChartEditorFunctionLibraly : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	static void AddGameplayTagDataTableToSettings(const UDataTable& DataTable);
	static UDataTable* CreateGameplayTagDataTable(const UDataTable& DataTable);
	static UChartDataAsset* CreateChartDataAsset(UDataTable& DataTable, const UDataTable& GameplayTagDataTable);
    static bool CheckIsSourceDataTable(const UDataTable& DataTable);
	static bool CheckOutFileIfNeed(const FString& InFile, bool bSilent);
	static const FName DUMMY;
};
