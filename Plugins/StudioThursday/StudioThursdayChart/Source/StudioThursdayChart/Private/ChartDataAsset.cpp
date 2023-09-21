// Copyright StudioThursday, Inc. All Rights Reserved.


#include "ChartDataAsset.h"
#include "Engine/DataAsset.h"

//====
// Public
//====

#if WITH_EDITOR
void UChartDataAsset::Build()
{
	if (!DataTable) { return; }
	if (!GameplayTagDataTable) { return; }
	RowStruct = DataTable->RowStruct;
	ClearMap();
	TArray RowNames = DataTable->GetRowNames();
	RowMap.Reserve(RowNames.Num());
	for (FName RowName : RowNames)
	{
		uint8* DataTableRow = DataTable->FindRowUnchecked(RowName);
		uint8* MapRow = (uint8*)FMemory::Malloc(RowStruct->GetStructureSize());
		RowStruct->InitializeStruct(MapRow);
		RowStruct->CopyScriptStruct(MapRow, DataTableRow);
		FGameplayTagTableRow* GameplayTagTableRow = GameplayTagDataTable->FindRow<FGameplayTagTableRow>(RowName, "");
		if (GameplayTagTableRow)
		{
			RowMap.Add(FGameplayTag::RequestGameplayTag(GameplayTagTableRow->Tag), MapRow);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("NoTagFound: %s"), *RowName.ToString());
		}
	}
	RowMap.GenerateValueArray(RowArray);
}
#endif

TArray<FGameplayTag> UChartDataAsset::GetRowTags() const
{
	TArray<FGameplayTag> Keys;
	RowMap.GetKeys(Keys);
	return Keys;
}

uint8* UChartDataAsset::GetData(const FGameplayTag& Tag) const
{
	uint8* const* RowDataPtr = RowMap.Find(Tag);
	return *RowDataPtr;
}

uint8* UChartDataAsset::GetDataByIndex(const int Index) const
{
	return RowArray[Index];
}

const TMap<FGameplayTag, uint8*>& UChartDataAsset::GetRowMap() const
{
	return RowMap;
}

UScriptStruct* UChartDataAsset::GetRowStruct() const
{
	return RowStruct;
}

bool UChartDataAsset::ContainsMap(const FName& TagName) const
{
	FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TagName, false);
	return RowMap.Contains(Tag);
}

void UChartDataAsset::Serialize(FStructuredArchiveRecord Record)
{
	Super::Serialize(Record);

	FArchive& BaseArchive = Record.GetUnderlyingArchive();

	if (RowStruct && RowStruct->HasAnyFlags(RF_NeedLoad))
	{
		auto RowStructLinker = RowStruct->GetLinker();
		if (RowStructLinker)
		{
			RowStructLinker->Preload(RowStruct);
		}
	}

	if (BaseArchive.IsLoading())
	{
		ClearMap();
		LoadStructData(Record.EnterField(TEXT("Data")));
	}
	else if (BaseArchive.IsSaving())
	{
		SaveStructData(Record.EnterField(TEXT("Data")));
	}
}

//====
// Private
//====

const UScriptStruct& UChartDataAsset::GetEmptyUsingStruct() const
{
	UScriptStruct* EmptyUsingStruct = RowStruct;
	if (!EmptyUsingStruct)
	{
		UE_LOG(LogTemp, Error, TEXT("Missing RowStruct while emptying ChartDataAsset '%s'!"), *GetPathName());
		EmptyUsingStruct = FTableRowBase::StaticStruct();
	}

	return *EmptyUsingStruct;
}

void UChartDataAsset::ClearMap()
{
	const UScriptStruct& EmptyUsingStruct = GetEmptyUsingStruct();

	// Iterate over all rows in table and free mem
	for (auto RowIt = RowMap.CreateIterator(); RowIt; ++RowIt)
	{
		uint8* RowData = RowIt.Value();
		EmptyUsingStruct.DestroyStruct(RowData);
		FMemory::Free(RowData);
	}

	// Finally empty the map
	RowMap.Empty();
	RowArray.Empty();
}

void UChartDataAsset::SaveStructData(FStructuredArchiveSlot Slot)
{
	UScriptStruct* SaveUsingStruct = RowStruct;
	if (!SaveUsingStruct)
	{
		UE_LOG(LogTemp, Error, TEXT("Missing RowStruct while saving ChartDataAsset '%s'!"), *GetPathName());
		SaveUsingStruct = FTableRowBase::StaticStruct();
	}

	int32 NumRows = RowMap.Num();
	FStructuredArchiveArray Array = Slot.EnterArray(NumRows);

	// Now iterate over rows in the map
	for (const auto& Entry : RowMap)
	{
		// Save out name
		FGameplayTag RowKey = Entry.Key;
		FStructuredArchiveRecord Row = Array.EnterElement().EnterRecord();
		Row << SA_VALUE(TEXT("Key"), RowKey);

		// Save out data
		uint8* RowData = Entry.Value;

		SaveUsingStruct->SerializeItem(Row.EnterField(TEXT("Value")), RowData, nullptr);
	}
}

void UChartDataAsset::LoadStructData(FStructuredArchiveSlot Slot)
{
	UScriptStruct* LoadUsingStruct = RowStruct;
	if (!LoadUsingStruct)
	{
		UE_LOG(LogTemp, Error, TEXT("Missing RowStruct while loading LikedPrimaryDataAsset '%s'!"), *GetPathName());
		LoadUsingStruct = FTableRowBase::StaticStruct();
	}

	int32 NumRows;
	FStructuredArchiveArray Array = Slot.EnterArray(NumRows);

	RowMap.Reserve(NumRows);
	for (int32 RowIdx = 0; RowIdx < NumRows; RowIdx++)
	{
		FStructuredArchiveRecord RowRecord = Array.EnterElement().EnterRecord();

		// Load row name
		FGameplayTag RowKey;
		RowRecord << SA_VALUE(TEXT("Key"), RowKey);

		// Load row data
		uint8* RowData = (uint8*)FMemory::Malloc(LoadUsingStruct->GetStructureSize());

		// And be sure to call DestroyScriptStruct later
		LoadUsingStruct->InitializeStruct(RowData);

		LoadUsingStruct->SerializeItem(RowRecord.EnterField(TEXT("Value")), RowData, nullptr);

		// Add to map
		RowMap.Add(RowKey, RowData);
	}
	RowMap.GenerateValueArray(RowArray);
}
