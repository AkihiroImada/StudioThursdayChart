// Copyright StudioThursday, Inc. All Rights Reserved.

#include "StudioThursdayChartEditor.h"
#include "Modules/ModuleManager.h"
#include "ChartDataAsset.h"
#include "ChartDataAssetDetailCustomization.h"

#define LOCTEXT_NAMESPACE "FStudioThursdayChartEditorModule"

void FStudioThursdayChartEditorModule::StartupModule()
{
    FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    PropertyModule.RegisterCustomClassLayout(UChartDataAsset::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FChartDataAssetDetailCustomization::MakeInstance));
    PropertyModule.NotifyCustomizationModuleChanged();
}

void FStudioThursdayChartEditorModule::ShutdownModule()
{
    FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    PropertyModule.UnregisterCustomClassLayout(UChartDataAsset::StaticClass()->GetFName());
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FStudioThursdayChartEditorModule, StudioThursdayChartEditor );
