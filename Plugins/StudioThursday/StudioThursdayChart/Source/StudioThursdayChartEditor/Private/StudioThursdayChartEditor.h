// Copyright StudioThursday, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FStudioThursdayChartEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
