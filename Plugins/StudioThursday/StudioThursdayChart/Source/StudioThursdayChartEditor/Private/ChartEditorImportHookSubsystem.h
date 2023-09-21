// Copyright StudioThursday, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/ImportSubsystem.h"
#include "ChartEditorImportHookSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class STUDIOTHURSDAYCHARTEDITOR_API UChartEditorImportHookSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()
public:
    void Initialize(FSubsystemCollectionBase& Collection) override;
    void Deinitialize() override;

private:
    void OnAssetPreImportFunc(UFactory* InFactory, UClass* InClass, UObject* InParent, const FName& Name, const TCHAR* Type);
    void OnAssetPostImportFunc(UFactory* InFactory, UObject* InCreatedObject);
    void FOnAssetReimportFunc(UObject* InCreatedObject);
    void FOnAssetPostLODImportFunc(UObject* InObject, int32 inLODIndex);
};
