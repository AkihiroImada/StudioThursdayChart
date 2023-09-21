// Copyright StudioThursday, Inc. All Rights Reserved.


#include "ChartFunctionLibrary.h"

int32 UChartFunctionLibrary::GetRowNum(const UChartDataAsset* ChartDataAsset)
{
	return ChartDataAsset->GetRowMap().Num();
}

int32 UChartFunctionLibrary::GetLastRowIndex(const UChartDataAsset* ChartDataAsset)
{
	return GetRowNum(ChartDataAsset) - 1;
}

bool UChartFunctionLibrary::Generic_GetChartDataAssetRow(const UChartDataAsset* ChartDataAsset, const FGameplayTag& RowKey, void* OutRowPtr)
{
	bool bFoundRow = false;

	if (OutRowPtr && ChartDataAsset)
	{
		void* RowPtr = ChartDataAsset->GetData(RowKey);

		if (RowPtr != nullptr)
		{
			const UScriptStruct* StructType = ChartDataAsset->GetRowStruct();

			if (StructType != nullptr)
			{
				StructType->CopyScriptStruct(OutRowPtr, RowPtr);
				bFoundRow = true;
			}
		}
	}

	return bFoundRow;
}

bool UChartFunctionLibrary::GetChartDataAssetRow(UChartDataAsset* ChartDataAsset, FGameplayTag RowKey, FTableRowBase& OutRow)
{
	// We should never hit this!  stubs to avoid NoExport on the class.
	check(0);
	return false;
}

bool UChartFunctionLibrary::Generic_GetChartDataAssetRowByIndex(const UChartDataAsset* ChartDataAsset, const int32 Index, void* OutRowPtr)
{
	bool bFoundRow = false;

	if (OutRowPtr && ChartDataAsset)
	{
		void* RowPtr = ChartDataAsset->GetDataByIndex(Index);

		if (RowPtr != nullptr)
		{
			const UScriptStruct* StructType = ChartDataAsset->GetRowStruct();

			if (StructType != nullptr)
			{
				StructType->CopyScriptStruct(OutRowPtr, RowPtr);
				bFoundRow = true;
			}
		}
	}

	return bFoundRow;
}

bool UChartFunctionLibrary::GetChartDataAssetRowByIndex(UChartDataAsset* ChartDataAsset, const int32 Index, FTableRowBase& OutRow)
{
	// We should never hit this!  stubs to avoid NoExport on the class.
	check(0);
	return false;
}
