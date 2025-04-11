// Copyright StudioThursday, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagsManager.h"
#include "ChartDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class STUDIOTHURSDAYCHART_API UChartDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Chart Runtime")
		TObjectPtr<UDataTable> DataTable;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Chart Runtime")
		TObjectPtr<const UDataTable> GameplayTagDataTable;
#if WITH_EDITOR
		void Build();
#endif
		UFUNCTION(BlueprintCallable, Category = "ChartDataAsset")
		TArray<FGameplayTag> GetRowTags() const;
		uint8* GetData(const FGameplayTag& Tag) const;
		uint8* GetDataByIndex(const int Index) const;
		const TMap<FGameplayTag, uint8*>& GetRowMap() const;
		UScriptStruct* GetRowStruct() const;
		bool ContainsMap(const FName& TagName) const;
		void Serialize(FStructuredArchiveRecord Record) override;
private:
	const UScriptStruct& GetEmptyUsingStruct() const;
	void ClearMap();
	void SaveStructData(FStructuredArchiveSlot Slot);
	void LoadStructData(FStructuredArchiveSlot Slot);

	UPROPERTY(EditAnywhere, Category = "Chart Runtime")
	TObjectPtr<UScriptStruct> RowStruct;
	TMap<FGameplayTag, uint8*> RowMap;
	// Use for optimize only.
	TArray<uint8*> RowArray;
};
