// Copyright StudioThursday, Inc. All Rights Reserved.

#include "ChartDataAssetDetailCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "SlateFwd.h"
#include "Widgets/Input/SNumericEntryBox.h"

#define LOCTEXT_NAMESPACE "ChartDataAsset"

TSharedRef<IDetailCustomization> FChartDataAssetDetailCustomization::MakeInstance()
{
    return MakeShareable(new FChartDataAssetDetailCustomization);
}

void FChartDataAssetDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailLayout.GetObjectsBeingCustomized(Objects);
	check(Objects.Num() > 0);
	ChartDataAsset = CastChecked<UChartDataAsset>(Objects[0].Get());
	{
		IDetailCategoryBuilder& CategoryBuilder = DetailLayout.EditCategory(TEXT("Chart Exec"));
		CategoryBuilder.AddCustomRow(FText::GetEmpty())
			.NameContent()[SNew(SButton)
			.Text(LOCTEXT("","Build From DataTable"))
			.OnClicked_Raw(this, &FChartDataAssetDetailCustomization::OnClickedButton)];
	}
	{
		IDetailCategoryBuilder& CategoryBuilder = DetailLayout.EditCategory(TEXT("Chart Entries"));

		for (const auto& Entry : ChartDataAsset->GetRowMap())
		{
			CategoryBuilder.AddCustomRow(FText::GetEmpty())
				.NameContent()[SNew(STextBlock).Text(FText::FromName(Entry.Key.GetTagName()))];
		}
	}
	auto CategorySorter = [](const TMap<FName, IDetailCategoryBuilder*>& Categories)
	{
		int32 Order = 0;
		auto SafeSetOrder = [&Categories, &Order](const FName& CategoryName)
		{
			if (IDetailCategoryBuilder* const* Builder = Categories.Find(CategoryName))
			{
				(*Builder)->SetSortOrder(Order++);
			}
		};

		SafeSetOrder(FName("Chart Exec"));
		SafeSetOrder(FName("Chart Development"));
		SafeSetOrder(FName("Chart Runtime"));
		SafeSetOrder(FName("Chart Entries"));
	};
	DetailLayout.SortCategories(CategorySorter);
}

void FChartDataAssetDetailCustomization::CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder)
{
	Layout = DetailBuilder;
	CustomizeDetails(*DetailBuilder);
}

TOptional<int32> FChartDataAssetDetailCustomization::GetValue() const
{
    return 0;
}

FReply FChartDataAssetDetailCustomization::OnClickedButton()
{
	
	if (ChartDataAsset.IsValid())
	{
		ChartDataAsset.Get()->Build();
	}
	if (IDetailLayoutBuilder* DetailBuilder = Layout.Pin().Get())
	{
		DetailBuilder->ForceRefreshDetails();
	}
	return FReply::Handled();
}
#undef LOCTEXT_NAMESPACE
