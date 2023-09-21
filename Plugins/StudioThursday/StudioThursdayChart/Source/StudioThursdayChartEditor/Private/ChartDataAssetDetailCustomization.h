// Copyright StudioThursday, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "ChartDataAsset.h"

class IDetailLayoutBuilder;

class FChartDataAssetDetailCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
	void CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder) override;
private:
	FReply OnClickedButton();
	TWeakPtr<IDetailLayoutBuilder> Layout;
	TWeakObjectPtr<UChartDataAsset> ChartDataAsset;
	TOptional<int32>    GetValue() const;
};
