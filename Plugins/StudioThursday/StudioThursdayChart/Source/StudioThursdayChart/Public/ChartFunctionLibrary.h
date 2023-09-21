// Copyright StudioThursday, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/FieldPathProperty.h"
#include "UObject/UnrealType.h"
#include "GameplayTagsManager.h"
#include "ChartDataAsset.h"
#include "ChartFunctionLibrary.generated.h"

class UChartDataAsset;

/**
 * 
 */
UCLASS()
class STUDIOTHURSDAYCHART_API UChartFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category = "ChartFunctionLibrary")
	static int32 GetRowNum(const UChartDataAsset* ChartDataAsset);
	UFUNCTION(BlueprintPure, Category = "ChartFunctionLibrary")
	static int32 GetLastRowIndex(const UChartDataAsset* ChartDataAsset);

	static bool Generic_GetChartDataAssetRow(const UChartDataAsset* ChartDataAsset, const FGameplayTag& RowKey, void* OutRowPtr);
    UFUNCTION(BlueprintCallable, CustomThunk, Category = "ChartFunctionLibrary", meta=(CustomStructureParam = "OutRow", BlueprintInternalUseOnly="true"))
    static bool GetChartDataAssetRow(UChartDataAsset* ChartDataAsset, FGameplayTag RowKey, FTableRowBase& OutRow);
    DECLARE_FUNCTION(execGetChartDataAssetRow)
    {
		P_GET_OBJECT(UChartDataAsset, ChartDataAsset);
		P_GET_STRUCT(FGameplayTag, RowKey);

		Stack.StepCompiledIn<FStructProperty>(NULL);
		void* OutRowPtr = Stack.MostRecentPropertyAddress;

		P_FINISH;
		bool bSuccess = false;

		FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
		if (!ChartDataAsset)
		{
			FBlueprintExceptionInfo ExceptionInfo(
				EBlueprintExceptionType::AccessViolation,
				NSLOCTEXT("GetDataTableRow", "MissingChartDataAssetInput", "Failed to resolve the ChartDataAsset input. Be sure the ChartDataAsset is valid.")
			);
			FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
		}
		else if (StructProp && OutRowPtr)
		{
			UScriptStruct* OutputType = StructProp->Struct;
			const UScriptStruct* InputRowType = ChartDataAsset->GetRowStruct();

			const bool bCompatible = (OutputType == InputRowType) ||
				(OutputType->IsChildOf(InputRowType) && FStructUtils::TheSameLayout(OutputType, InputRowType));
			if (bCompatible)
			{
				P_NATIVE_BEGIN;
				bSuccess = Generic_GetChartDataAssetRow(ChartDataAsset, RowKey, OutRowPtr);
				P_NATIVE_END;
			}
			else
			{
				FBlueprintExceptionInfo ExceptionInfo(
					EBlueprintExceptionType::AccessViolation,
					NSLOCTEXT("GetChartDataAssetRow", "IncompatibleProperty", "Incompatible output parameter; the data ChartDataAsset's type is not the same as the return type.")
				);
				FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
			}
		}
		else
		{
			FBlueprintExceptionInfo ExceptionInfo(
				EBlueprintExceptionType::AccessViolation,
				NSLOCTEXT("GetChartDataAssetRow", "MissingOutputProperty", "Failed to resolve the output parameter for GetChartDataAssetRow.")
			);
			FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
		}
		*(bool*)RESULT_PARAM = bSuccess;
    }

	static bool Generic_GetChartDataAssetRowByIndex(const UChartDataAsset* ChartDataAsset, const int32 Index, void* OutRowPtr);
    UFUNCTION(BlueprintCallable, CustomThunk, Category = "ChartFunctionLibrary", meta=(CustomStructureParam = "OutRow", BlueprintInternalUseOnly="true"))
    static bool GetChartDataAssetRowByIndex(UChartDataAsset* ChartDataAsset, const int32 Index, FTableRowBase& OutRow);
	DECLARE_FUNCTION(execGetChartDataAssetRowByIndex)
	{
		P_GET_OBJECT(UChartDataAsset, ChartDataAsset);
		P_GET_PROPERTY(FIntProperty, Index)

		Stack.StepCompiledIn<FStructProperty>(NULL);
		void* OutRowPtr = Stack.MostRecentPropertyAddress;

		P_FINISH;
		bool bSuccess = false;

		FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
		if (!ChartDataAsset)
		{
			FBlueprintExceptionInfo ExceptionInfo(
				EBlueprintExceptionType::AccessViolation,
				NSLOCTEXT("GetDataTableRowByIndex", "MissingChartDataAssetInput", "Failed to resolve the ChartDataAsset input. Be sure the ChartDataAsset is valid.")
			);
			FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
		}
		else if (StructProp && OutRowPtr)
		{
			UScriptStruct* OutputType = StructProp->Struct;
			const UScriptStruct* InputRowType = ChartDataAsset->GetRowStruct();

			const bool bCompatible = (OutputType == InputRowType) ||
				(OutputType->IsChildOf(InputRowType) && FStructUtils::TheSameLayout(OutputType, InputRowType));
			if (bCompatible)
			{
				P_NATIVE_BEGIN;
				bSuccess = Generic_GetChartDataAssetRowByIndex(ChartDataAsset, Index, OutRowPtr);
				P_NATIVE_END;
			}
			else
			{
				FBlueprintExceptionInfo ExceptionInfo(
					EBlueprintExceptionType::AccessViolation,
					NSLOCTEXT("GetChartDataAssetRowByIndex", "IncompatibleProperty", "Incompatible output parameter; the data ChartDataAsset's type is not the same as the return type.")
				);
				FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
			}
		}
		else
		{
			FBlueprintExceptionInfo ExceptionInfo(
				EBlueprintExceptionType::AccessViolation,
				NSLOCTEXT("GetChartDataAssetRowByIndex", "MissingOutputProperty", "Failed to resolve the output parameter for GetChartDataAssetRowByIndex.")
			);
			FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
		}
		*(bool*)RESULT_PARAM = bSuccess;
	}
};
