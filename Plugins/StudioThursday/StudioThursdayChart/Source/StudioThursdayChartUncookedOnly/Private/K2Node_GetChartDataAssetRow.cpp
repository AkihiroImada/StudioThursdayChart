// Copyright StudioThursday, Inc. All Rights Reserved.

#include "K2Node_GetChartDataAssetRow.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "Containers/EnumAsByte.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "DataTableEditorUtils.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphSchema.h"
#include "EdGraphSchema_K2.h"
#include "EditorCategoryUtils.h"
#include "Engine/DataTable.h"
#include "Engine/UserDefinedEnum.h"
#include "Engine/MemberReference.h"
#include "HAL/PlatformMath.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Regex.h"
#include "K2Node_CallFunction.h"
#include "K2Node_IfThenElse.h"
#include "Kismet/DataTableFunctionLibrary.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/CompilerResultsLog.h"
#include "KismetCompiler.h"
#include "Math/Color.h"
#include "Misc/AssertionMacros.h"
#include "Styling/AppStyle.h"
#include "Templates/Casts.h"
#include "Templates/ChooseClass.h"
#include "UObject/Class.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"
#include "UObject/ObjectPtr.h"
#include "UObject/UObjectBaseUtility.h"
#include "UObject/UnrealNames.h"
#include "UObject/WeakObjectPtr.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "ChartDataAsset.h"
#include "ChartFunctionLibrary.h"

class UBlueprint;

#define LOCTEXT_NAMESPACE "K2Node_GetChartDataAssetRow"

namespace GetChartDataAssetRowHelper
{
	const FName ChartDataAssetPinName = "ChartDataAsset";
	const FName RowNotFoundPinName = "RowNotFound";
	const FName RowKeyPinName = "RowKey";
}

UK2Node_GetChartDataAssetRow::UK2Node_GetChartDataAssetRow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeTooltip = LOCTEXT("NodeTooltip", "Attempts to retrieve a Row from a Chart via GameplayTag");
}

void UK2Node_GetChartDataAssetRow::AllocateDefaultPins()
{
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

	// Add execution pins
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	UEdGraphPin* RowFoundPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
	RowFoundPin->PinFriendlyName = LOCTEXT("GetChartDataAssetRow Row Found Exec pin", "Row Found");
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, GetChartDataAssetRowHelper::RowNotFoundPinName);

	UEdGraphPin* ChartDataAssetPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UChartDataAsset::StaticClass(), GetChartDataAssetRowHelper::ChartDataAssetPinName);
	SetPinToolTip(*ChartDataAssetPin, LOCTEXT("ChartDataAssetPinDescription", "The ChartDataAsset you want to retreive a row from"));
			
	UEdGraphPin* RowKeyPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Struct, FGameplayTag::StaticStruct(), GetChartDataAssetRowHelper::RowKeyPinName);
	SetPinToolTip(*RowKeyPin, LOCTEXT("RowKeyPinDescription", "The key of the row to retrieve from the ChartDataAsset"));

	UEdGraphPin* ResultPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, UEdGraphSchema_K2::PN_ReturnValue);
	ResultPin->PinFriendlyName = LOCTEXT("GetChartDataAssetRow Output Row", "Out Row");
	SetPinToolTip(*ResultPin, LOCTEXT("ResultPinDescription", "The returned TableRow, if found"));

	Super::AllocateDefaultPins();
}

void UK2Node_GetChartDataAssetRow::SetPinToolTip(UEdGraphPin& MutatablePin, const FText& PinDescription) const
{
	MutatablePin.PinToolTip = UEdGraphSchema_K2::TypeToText(MutatablePin.PinType).ToString();

	UEdGraphSchema_K2 const* const K2Schema = Cast<const UEdGraphSchema_K2>(GetSchema());
	if (K2Schema != nullptr)
	{
		MutatablePin.PinToolTip += TEXT(" ");
		MutatablePin.PinToolTip += K2Schema->GetPinDisplayName(&MutatablePin).ToString();
	}

	MutatablePin.PinToolTip += FString(TEXT("\n")) + PinDescription.ToString();
}

void UK2Node_GetChartDataAssetRow::RefreshOutputPinType()
{
	UScriptStruct* OutputType = GetChartDataAssetRowStructType();
	SetReturnTypeForStruct(OutputType);
}

void UK2Node_GetChartDataAssetRow::SetReturnTypeForStruct(UScriptStruct* NewRowStruct)
{
	UScriptStruct* OldRowStruct = GetReturnTypeForStruct();
	if (NewRowStruct != OldRowStruct)
	{
		UEdGraphPin* ResultPin = GetResultPin();

		if (ResultPin->SubPins.Num() > 0)
		{
			GetSchema()->RecombinePin(ResultPin);
		}

		// NOTE: purposefully not disconnecting the ResultPin (even though it changed type)... we want the user to see the old
		//       connections, and incompatible connections will produce an error (plus, some super-struct connections may still be valid)
		ResultPin->PinType.PinSubCategoryObject = NewRowStruct;
		ResultPin->PinType.PinCategory = (NewRowStruct == nullptr) ? UEdGraphSchema_K2::PC_Wildcard : UEdGraphSchema_K2::PC_Struct;

		CachedNodeTitle.Clear();
	}
}

UScriptStruct* UK2Node_GetChartDataAssetRow::GetReturnTypeForStruct()
{
	UScriptStruct* ReturnStructType = (UScriptStruct*)(GetResultPin()->PinType.PinSubCategoryObject.Get());

	return ReturnStructType;
}

UScriptStruct* UK2Node_GetChartDataAssetRow::GetChartDataAssetRowStructType() const
{
	UScriptStruct* RowStructType = nullptr;

	UEdGraphPin* ChartDataAssetPin = GetChartDataAssetPin();
	if(ChartDataAssetPin && ChartDataAssetPin->DefaultObject != nullptr && ChartDataAssetPin->LinkedTo.Num() == 0)
	{
		if (const UChartDataAsset* ChartDataAsset = Cast<const UChartDataAsset>(ChartDataAssetPin->DefaultObject))
		{
			RowStructType = ChartDataAsset->GetRowStruct();
		}
	}

	if (RowStructType == nullptr)
	{
		UEdGraphPin* ResultPin = GetResultPin();
		if (ResultPin && ResultPin->LinkedTo.Num() > 0)
		{
			RowStructType = Cast<UScriptStruct>(ResultPin->LinkedTo[0]->PinType.PinSubCategoryObject.Get());
			if (RowStructType == nullptr && ResultPin->PinType.PinCategory != UEdGraphSchema_K2::PC_Wildcard)
			{
				RowStructType = GetFallbackStruct();
			}
			for (int32 LinkIndex = 1; LinkIndex < ResultPin->LinkedTo.Num(); ++LinkIndex)
			{
				UEdGraphPin* Link = ResultPin->LinkedTo[LinkIndex];
				UScriptStruct* LinkType = Cast<UScriptStruct>(Link->PinType.PinSubCategoryObject.Get());

				if (RowStructType && RowStructType->IsChildOf(LinkType))
				{
					RowStructType = LinkType;
				}
			}
		}
	}
	return RowStructType;
}

void UK2Node_GetChartDataAssetRow::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) 
{
	Super::ReallocatePinsDuringReconstruction(OldPins);

	if (UEdGraphPin* ChartDataAssetPin = GetChartDataAssetPin(&OldPins))
	{
		if (UChartDataAsset* ChartDataAsset = Cast<UChartDataAsset>(ChartDataAssetPin->DefaultObject))
		{
			// make sure to properly load the DataAsset object so that we can 
			// farm the "RowStruct" property from it (below, in GetChartDataAssetRowStructType)
			PreloadObject(ChartDataAsset);
		}
	}
}

void UK2Node_GetChartDataAssetRow::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	// actions get registered under specific object-keys; the idea is that 
	// actions might have to be updated (or deleted) if their object-key is  
	// mutated (or removed)... here we use the node's class (so if the node 
	// type disappears, then the action should go with it)
	UClass* ActionKey = GetClass();
	// to keep from needlessly instantiating a UBlueprintNodeSpawner, first   
	// check to make sure that the registrar is looking for actions of this type
	// (could be regenerating actions for a specific asset, and therefore the 
	// registrar would only accept actions corresponding to that asset)
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);

		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_GetChartDataAssetRow::GetMenuCategory() const
{
	return FEditorCategoryUtils::GetCommonCategory(FCommonEditorCategory::Utilities);
}

bool UK2Node_GetChartDataAssetRow::IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin, FString& OutReason) const
{
	if (MyPin == GetResultPin() && MyPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard)
	{
		bool bDisallowed = true;
		if (OtherPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Struct)
		{
			if (UScriptStruct* ConnectionType = Cast<UScriptStruct>(OtherPin->PinType.PinSubCategoryObject.Get()))
			{
				bDisallowed = !FDataTableEditorUtils::IsValidTableStruct(ConnectionType);
			}
		}
		else if (OtherPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard)
		{
			bDisallowed = false;
		}

		if (bDisallowed)
		{
			OutReason = TEXT("Must be a struct that can be used in a DataTable");
		}
		return bDisallowed;
	}
	return false;
}

void UK2Node_GetChartDataAssetRow::PinDefaultValueChanged(UEdGraphPin* ChangedPin) 
{
	if (ChangedPin && ChangedPin->PinName == GetChartDataAssetRowHelper::ChartDataAssetPinName)
	{
		RefreshOutputPinType();

		UEdGraphPin* RowKeyPin = GetRowKeyPin();
		UChartDataAsset* ChartDataAsset = Cast<UChartDataAsset>(ChangedPin->DefaultObject);
		if (RowKeyPin)
		{
			if (ChartDataAsset && (RowKeyPin->DefaultValue.IsEmpty() || !IsRowKeyContainsRowMap(*ChartDataAsset)))
			{
				if (auto Iterator = ChartDataAsset->GetRowMap().CreateConstIterator())
				{
					RowKeyPin->DefaultValue = FString(TEXT(""));
				}
			}	
		}
	}
}

FText UK2Node_GetChartDataAssetRow::GetTooltipText() const
{
	return NodeTooltip;
}

UEdGraphPin* UK2Node_GetChartDataAssetRow::GetThenPin()const
{
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

	UEdGraphPin* Pin = FindPinChecked(UEdGraphSchema_K2::PN_Then);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

UEdGraphPin* UK2Node_GetChartDataAssetRow::GetChartDataAssetPin(const TArray<UEdGraphPin*>* InPinsToSearch /*= NULL*/) const
{
	const TArray<UEdGraphPin*>* PinsToSearch = InPinsToSearch ? InPinsToSearch : &Pins;
    
	UEdGraphPin* Pin = nullptr;
	for (UEdGraphPin* TestPin : *PinsToSearch)
	{
		if (TestPin && TestPin->PinName == GetChartDataAssetRowHelper::ChartDataAssetPinName)
		{
			Pin = TestPin;
			break;
		}
	}
	check(Pin == nullptr || Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_GetChartDataAssetRow::GetRowKeyPin() const
{
	UEdGraphPin* Pin = FindPinChecked(GetChartDataAssetRowHelper::RowKeyPinName);
	check(Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_GetChartDataAssetRow::GetRowNotFoundPin() const
{
	UEdGraphPin* Pin = FindPinChecked(GetChartDataAssetRowHelper::RowNotFoundPinName);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

UEdGraphPin* UK2Node_GetChartDataAssetRow::GetResultPin() const
{
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

	UEdGraphPin* Pin = FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

FText UK2Node_GetChartDataAssetRow::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (TitleType == ENodeTitleType::MenuTitle)
	{
		return LOCTEXT("ListViewTitle", "Get Chart Data Asset Row");
	}
	else if (UEdGraphPin* ChartPin = GetChartDataAssetPin())
	{
		if (ChartPin->LinkedTo.Num() > 0)
		{
			return NSLOCTEXT("K2Node", "ChartDataAsset_Title_Unknown", "Get Chart Data Asset Row");
		}
		else if (ChartPin->DefaultObject == nullptr)
		{
			return NSLOCTEXT("K2Node", "ChartDataAsset_Title_None", "Get Chart Data Asset Row NONE");
		}
		else if (CachedNodeTitle.IsOutOfDate(this))
		{
			FFormatNamedArguments Args;
			Args.Add(TEXT("ChartName"), FText::FromString(ChartPin->DefaultObject->GetName()));

			FText LocFormat = NSLOCTEXT("K2Node", "Chart", "Get Chart Data Asset Row {ChartName}");
			// FText::Format() is slow, so we cache this to save on performance
			CachedNodeTitle.SetCachedText(FText::Format(LocFormat, Args), this);
		}
	}
	else
	{
		return NSLOCTEXT("K2Node", "ChartDataAsset_Title_None", "Get Chart Data Asset Row NONE");
	}	
	return CachedNodeTitle;
}

void UK2Node_GetChartDataAssetRow::ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    Super::ExpandNode(CompilerContext, SourceGraph);
    
    UEdGraphPin* OriginalChartDataAsset = GetChartDataAssetPin();
	UChartDataAsset* ChartDataAsset = (OriginalChartDataAsset != NULL) ? Cast<UChartDataAsset>(OriginalChartDataAsset->DefaultObject) : NULL;
    if((nullptr == OriginalChartDataAsset) || (0 == OriginalChartDataAsset->LinkedTo.Num() && nullptr == ChartDataAsset))
    {
        CompilerContext.MessageLog.Error(*LOCTEXT("GetDataChartDataAssetRowNoChart_Error", "GetChartDataAssetRow must have a ChartDataAsset specified.").ToString(), this);
        // we break exec links so this is the only error we get
        BreakAllNodeLinks();
        return;
    }

	// FUNCTION NODE
	const FName FunctionName = GET_FUNCTION_NAME_CHECKED(UChartFunctionLibrary, GetChartDataAssetRow);
	UK2Node_CallFunction* GetChartDataAssetRowFunction = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	GetChartDataAssetRowFunction->FunctionReference.SetExternalMember(FunctionName, UChartFunctionLibrary::StaticClass());
	GetChartDataAssetRowFunction->AllocateDefaultPins();
    CompilerContext.MovePinLinksToIntermediate(*GetExecPin(), *(GetChartDataAssetRowFunction->GetExecPin()));

	// Connect the input of our GetChartDataAssetRow to the Input of our Function pin
    UEdGraphPin* ChartInPin = GetChartDataAssetRowFunction->FindPinChecked(TEXT("ChartDataAsset"));
	if(OriginalChartDataAsset->LinkedTo.Num() > 0)
	{
		// Copy the connection
		CompilerContext.MovePinLinksToIntermediate(*OriginalChartDataAsset, *ChartInPin);
	}
	else
	{
		// Copy literal
		ChartInPin->DefaultObject = OriginalChartDataAsset->DefaultObject;
	}

	UEdGraphPin* RowKeyPin = GetRowKeyPin();
	UEdGraphPin* RowKeyInPin = GetChartDataAssetRowFunction->FindPinChecked(TEXT("RowKey"));
	RowKeyInPin->PinType = RowKeyPin->PinType;
	CompilerContext.MovePinLinksToIntermediate(*RowKeyPin, *RowKeyInPin);

	// Get some pins to work with
	UEdGraphPin* OriginalOutRowPin = FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue);
	UEdGraphPin* FunctionOutRowPin = GetChartDataAssetRowFunction->FindPinChecked(TEXT("OutRow"));
    UEdGraphPin* FunctionReturnPin = GetChartDataAssetRowFunction->FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue);
    UEdGraphPin* FunctionThenPin = GetChartDataAssetRowFunction->GetThenPin();
        
    // Set the type of the OutRow pin on this expanded mode to match original
    FunctionOutRowPin->PinType = OriginalOutRowPin->PinType;
	FunctionOutRowPin->PinType.PinSubCategoryObject = OriginalOutRowPin->PinType.PinSubCategoryObject;
        
    //BRANCH NODE
    UK2Node_IfThenElse* BranchNode = CompilerContext.SpawnIntermediateNode<UK2Node_IfThenElse>(this, SourceGraph);
    BranchNode->AllocateDefaultPins();
    // Hook up inputs to branch
    FunctionThenPin->MakeLinkTo(BranchNode->GetExecPin());
    FunctionReturnPin->MakeLinkTo(BranchNode->GetConditionPin());
        
    // Hook up outputs
    CompilerContext.MovePinLinksToIntermediate(*GetThenPin(), *(BranchNode->GetThenPin()));
    CompilerContext.MovePinLinksToIntermediate(*GetRowNotFoundPin(), *(BranchNode->GetElsePin()));
    CompilerContext.MovePinLinksToIntermediate(*OriginalOutRowPin, *FunctionOutRowPin);

	BreakAllNodeLinks();
}

FSlateIcon UK2Node_GetChartDataAssetRow::GetIconAndTint(FLinearColor& OutColor) const
{
	OutColor = GetNodeTitleColor();
	static FSlateIcon Icon(FAppStyle::GetAppStyleSetName(), "Kismet.AllClasses.FunctionIcon");
	return Icon;
}

void UK2Node_GetChartDataAssetRow::PostReconstructNode()
{
	Super::PostReconstructNode();

	RefreshOutputPinType();
}

void UK2Node_GetChartDataAssetRow::EarlyValidation(class FCompilerResultsLog& MessageLog) const
{
	Super::EarlyValidation(MessageLog);

	const UEdGraphPin* ChartDataAssetPin = GetChartDataAssetPin();
	const UEdGraphPin* RowKeyPin = GetRowKeyPin();
	if (!ChartDataAssetPin || !RowKeyPin)
	{
		MessageLog.Error(*LOCTEXT("MissingPins", "Missing pins in @@").ToString(), this);
		return;
	}

	if (ChartDataAssetPin->LinkedTo.Num() == 0)
	{
		const UChartDataAsset* ChartDataAsset = Cast<UChartDataAsset>(ChartDataAssetPin->DefaultObject);
		if (!ChartDataAsset)
		{
			MessageLog.Error(*LOCTEXT("NoChart", "No Chart in @@").ToString(), this);
			return;
		}

		if (!RowKeyPin->LinkedTo.Num())
		{
			if (!IsRowKeyContainsRowMap(*ChartDataAsset))
			{
				const FString Msg = FText::Format(
					LOCTEXT("WrongRowNameFmt", "'{0}' tag is not stored in '{1}'. @@"),
					RowKeyPin->GetDefaultAsText(),
					FText::FromString(GetFullNameSafe(ChartDataAsset))
				).ToString();
				MessageLog.Error(*Msg, this);
				return;
			}
		}
	}	
}

void UK2Node_GetChartDataAssetRow::PreloadRequiredAssets()
{
	if (UEdGraphPin* ChartDataAssetPin = GetChartDataAssetPin())
	{
		if (UChartDataAsset* ChartDataAsset = Cast<UChartDataAsset>(ChartDataAssetPin->DefaultObject))
		{
			// make sure to properly load the DataAsset object so that we can 
			// farm the "RowStruct" property from it (below, in GetChartDataAssetRowStructType)
			PreloadObject(ChartDataAsset);
		}
	}
	return Super::PreloadRequiredAssets();
}

void UK2Node_GetChartDataAssetRow::NotifyPinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::NotifyPinConnectionListChanged(Pin);

	if (Pin == GetResultPin())
	{
		UEdGraphPin* ChartDataAssetPin = GetChartDataAssetPin();
		// this connection would only change the output type if the table pin is undefined
		const bool bIsTypeAuthority = (ChartDataAssetPin->LinkedTo.Num() > 0 || ChartDataAssetPin->DefaultObject == nullptr);
		if (bIsTypeAuthority)
		{
			RefreshOutputPinType();
		}		
	}
	else if (Pin == GetChartDataAssetPin())
	{
		const bool bConnectionAdded = Pin->LinkedTo.Num() > 0;
		if (bConnectionAdded)
		{
			// if the output connection was previously, incompatible, it now becomes the authority on this node's output type
			RefreshOutputPinType();
		}
	}
}

bool UK2Node_GetChartDataAssetRow::IsRowKeyContainsRowMap(const UChartDataAsset& ChartDataAsset) const
{
	const UEdGraphPin* RowKeyPin = GetRowKeyPin();
	const FRegexPattern Pattern = FRegexPattern(FString(TEXT("TagName=\"(.*)\"")));
	const FString RowKeyPinString = RowKeyPin->GetDefaultAsString();
	FRegexMatcher Matcher(Pattern, RowKeyPinString);
	while (Matcher.FindNext())
	{
		FName TagName = FName(Matcher.GetCaptureGroup(1));
		if (ChartDataAsset.ContainsMap(TagName))
		{
			return true;
		}
	}
	return false;
}

#undef LOCTEXT_NAMESPACE
