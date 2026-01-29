// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "MountainGenSettings.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
static_assert(!UE_WITH_CONSTINIT_UOBJECT, "This generated code can only be compiled with !UE_WITH_CONSTINIT_OBJECT");
void EmptyLinkFunctionForGeneratedCodeMountainGenSettings() {}

// ********** Begin Cross Module References ********************************************************
MOUNTAINGEN_API UEnum* Z_Construct_UEnum_MountainGen_EMountainGenDifficulty();
MOUNTAINGEN_API UScriptStruct* Z_Construct_UScriptStruct_FMGTargets();
MOUNTAINGEN_API UScriptStruct* Z_Construct_UScriptStruct_FMountainGenSettings();
UPackage* Z_Construct_UPackage__Script_MountainGen();
// ********** End Cross Module References **********************************************************

// ********** Begin Enum EMountainGenDifficulty ****************************************************
static FEnumRegistrationInfo Z_Registration_Info_UEnum_EMountainGenDifficulty;
static UEnum* EMountainGenDifficulty_StaticEnum()
{
	if (!Z_Registration_Info_UEnum_EMountainGenDifficulty.OuterSingleton)
	{
		Z_Registration_Info_UEnum_EMountainGenDifficulty.OuterSingleton = GetStaticEnum(Z_Construct_UEnum_MountainGen_EMountainGenDifficulty, (UObject*)Z_Construct_UPackage__Script_MountainGen(), TEXT("EMountainGenDifficulty"));
	}
	return Z_Registration_Info_UEnum_EMountainGenDifficulty.OuterSingleton;
}
template<> MOUNTAINGEN_NON_ATTRIBUTED_API UEnum* StaticEnum<EMountainGenDifficulty>()
{
	return EMountainGenDifficulty_StaticEnum();
}
struct Z_Construct_UEnum_MountainGen_EMountainGenDifficulty_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Enum_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "Easy.DisplayName", "Easy" },
		{ "Easy.Name", "EMountainGenDifficulty::Easy" },
		{ "Extreme.DisplayName", "Extreme" },
		{ "Extreme.Name", "EMountainGenDifficulty::Extreme" },
		{ "Hard.DisplayName", "Hard" },
		{ "Hard.Name", "EMountainGenDifficulty::Hard" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
		{ "Normal.DisplayName", "Normal" },
		{ "Normal.Name", "EMountainGenDifficulty::Normal" },
	};
#endif // WITH_METADATA
	static constexpr UECodeGen_Private::FEnumeratorParam Enumerators[] = {
		{ "EMountainGenDifficulty::Easy", (int64)EMountainGenDifficulty::Easy },
		{ "EMountainGenDifficulty::Normal", (int64)EMountainGenDifficulty::Normal },
		{ "EMountainGenDifficulty::Hard", (int64)EMountainGenDifficulty::Hard },
		{ "EMountainGenDifficulty::Extreme", (int64)EMountainGenDifficulty::Extreme },
	};
	static const UECodeGen_Private::FEnumParams EnumParams;
}; // struct Z_Construct_UEnum_MountainGen_EMountainGenDifficulty_Statics 
const UECodeGen_Private::FEnumParams Z_Construct_UEnum_MountainGen_EMountainGenDifficulty_Statics::EnumParams = {
	(UObject*(*)())Z_Construct_UPackage__Script_MountainGen,
	nullptr,
	"EMountainGenDifficulty",
	"EMountainGenDifficulty",
	Z_Construct_UEnum_MountainGen_EMountainGenDifficulty_Statics::Enumerators,
	RF_Public|RF_Transient|RF_MarkAsNative,
	UE_ARRAY_COUNT(Z_Construct_UEnum_MountainGen_EMountainGenDifficulty_Statics::Enumerators),
	EEnumFlags::None,
	(uint8)UEnum::ECppForm::EnumClass,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UEnum_MountainGen_EMountainGenDifficulty_Statics::Enum_MetaDataParams), Z_Construct_UEnum_MountainGen_EMountainGenDifficulty_Statics::Enum_MetaDataParams)
};
UEnum* Z_Construct_UEnum_MountainGen_EMountainGenDifficulty()
{
	if (!Z_Registration_Info_UEnum_EMountainGenDifficulty.InnerSingleton)
	{
		UECodeGen_Private::ConstructUEnum(Z_Registration_Info_UEnum_EMountainGenDifficulty.InnerSingleton, Z_Construct_UEnum_MountainGen_EMountainGenDifficulty_Statics::EnumParams);
	}
	return Z_Registration_Info_UEnum_EMountainGenDifficulty.InnerSingleton;
}
// ********** End Enum EMountainGenDifficulty ******************************************************

// ********** Begin ScriptStruct FMGTargets ********************************************************
struct Z_Construct_UScriptStruct_FMGTargets_Statics
{
	static inline consteval int32 GetStructSize() { return sizeof(FMGTargets); }
	static inline consteval int16 GetStructAlignment() { return alignof(FMGTargets); }
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveMin_MetaData[] = {
		{ "Category", "MountainGen|Targets" },
		{ "ClampMax", "1.0" },
		{ "ClampMin", "0.0" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// 0..1 : Cave void ratio target\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "0..1 : Cave void ratio target" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveMax_MetaData[] = {
		{ "Category", "MountainGen|Targets" },
		{ "ClampMax", "1.0" },
		{ "ClampMin", "0.0" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OverhangMin_MetaData[] = {
		{ "Category", "MountainGen|Targets" },
		{ "ClampMax", "1.0" },
		{ "ClampMin", "0.0" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// 0..1 : Overhang ratio target\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "0..1 : Overhang ratio target" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OverhangMax_MetaData[] = {
		{ "Category", "MountainGen|Targets" },
		{ "ClampMax", "1.0" },
		{ "ClampMin", "0.0" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_SteepMin_MetaData[] = {
		{ "Category", "MountainGen|Targets" },
		{ "ClampMax", "1.0" },
		{ "ClampMin", "0.0" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// 0..1 : Steep ratio target\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "0..1 : Steep ratio target" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_SteepMax_MetaData[] = {
		{ "Category", "MountainGen|Targets" },
		{ "ClampMax", "1.0" },
		{ "ClampMin", "0.0" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
#endif // WITH_METADATA

// ********** Begin ScriptStruct FMGTargets constinit property declarations ************************
	static const UECodeGen_Private::FFloatPropertyParams NewProp_CaveMin;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_CaveMax;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_OverhangMin;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_OverhangMax;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_SteepMin;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_SteepMax;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
// ********** End ScriptStruct FMGTargets constinit property declarations **************************
	static void* NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FMGTargets>();
	}
	static const UECodeGen_Private::FStructParams StructParams;
}; // struct Z_Construct_UScriptStruct_FMGTargets_Statics
static FStructRegistrationInfo Z_Registration_Info_UScriptStruct_FMGTargets;
class UScriptStruct* FMGTargets::StaticStruct()
{
	if (!Z_Registration_Info_UScriptStruct_FMGTargets.OuterSingleton)
	{
		Z_Registration_Info_UScriptStruct_FMGTargets.OuterSingleton = GetStaticStruct(Z_Construct_UScriptStruct_FMGTargets, (UObject*)Z_Construct_UPackage__Script_MountainGen(), TEXT("MGTargets"));
	}
	return Z_Registration_Info_UScriptStruct_FMGTargets.OuterSingleton;
	}

// ********** Begin ScriptStruct FMGTargets Property Definitions ***********************************
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMGTargets_Statics::NewProp_CaveMin = { "CaveMin", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMGTargets, CaveMin), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CaveMin_MetaData), NewProp_CaveMin_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMGTargets_Statics::NewProp_CaveMax = { "CaveMax", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMGTargets, CaveMax), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CaveMax_MetaData), NewProp_CaveMax_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMGTargets_Statics::NewProp_OverhangMin = { "OverhangMin", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMGTargets, OverhangMin), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_OverhangMin_MetaData), NewProp_OverhangMin_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMGTargets_Statics::NewProp_OverhangMax = { "OverhangMax", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMGTargets, OverhangMax), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_OverhangMax_MetaData), NewProp_OverhangMax_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMGTargets_Statics::NewProp_SteepMin = { "SteepMin", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMGTargets, SteepMin), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_SteepMin_MetaData), NewProp_SteepMin_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMGTargets_Statics::NewProp_SteepMax = { "SteepMax", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMGTargets, SteepMax), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_SteepMax_MetaData), NewProp_SteepMax_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FMGTargets_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMGTargets_Statics::NewProp_CaveMin,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMGTargets_Statics::NewProp_CaveMax,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMGTargets_Statics::NewProp_OverhangMin,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMGTargets_Statics::NewProp_OverhangMax,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMGTargets_Statics::NewProp_SteepMin,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMGTargets_Statics::NewProp_SteepMax,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FMGTargets_Statics::PropPointers) < 2048);
// ********** End ScriptStruct FMGTargets Property Definitions *************************************
const UECodeGen_Private::FStructParams Z_Construct_UScriptStruct_FMGTargets_Statics::StructParams = {
	(UObject* (*)())Z_Construct_UPackage__Script_MountainGen,
	nullptr,
	&NewStructOps,
	"MGTargets",
	Z_Construct_UScriptStruct_FMGTargets_Statics::PropPointers,
	UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FMGTargets_Statics::PropPointers),
	sizeof(FMGTargets),
	alignof(FMGTargets),
	RF_Public|RF_Transient|RF_MarkAsNative,
	EStructFlags(0x00000001),
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FMGTargets_Statics::Struct_MetaDataParams), Z_Construct_UScriptStruct_FMGTargets_Statics::Struct_MetaDataParams)
};
UScriptStruct* Z_Construct_UScriptStruct_FMGTargets()
{
	if (!Z_Registration_Info_UScriptStruct_FMGTargets.InnerSingleton)
	{
		UECodeGen_Private::ConstructUScriptStruct(Z_Registration_Info_UScriptStruct_FMGTargets.InnerSingleton, Z_Construct_UScriptStruct_FMGTargets_Statics::StructParams);
	}
	return CastChecked<UScriptStruct>(Z_Registration_Info_UScriptStruct_FMGTargets.InnerSingleton);
}
// ********** End ScriptStruct FMGTargets **********************************************************

// ********** Begin ScriptStruct FMountainGenSettings **********************************************
struct Z_Construct_UScriptStruct_FMountainGenSettings_Statics
{
	static inline consteval int32 GetStructSize() { return sizeof(FMountainGenSettings); }
	static inline consteval int16 GetStructAlignment() { return alignof(FMountainGenSettings); }
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Seed_MetaData[] = {
		{ "Category", "MountainGen|Main" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// ============================================================\n// 0) Reproducibility\n// ============================================================\n// Seed <= 0 : \xef\xbf\xbd\xda\xb5\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\n// Seed > 0  : \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "0) Reproducibility\n\nSeed <= 0 : \xef\xbf\xbd\xda\xb5\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\nSeed > 0  : \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_BaseHeightCm_MetaData[] = {
		{ "Category", "MountainGen|Main" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Base plane offset\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Base plane offset" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_HeightAmpCm_MetaData[] = {
		{ "Category", "MountainGen|Main" },
		{ "ClampMin", "1000.0" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xef\xbf\xbd\xd6\xb4\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xef\xbf\xbd\xd6\xb4\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_EnvelopeRadiusCm_MetaData[] = {
		{ "Category", "MountainGen|Main" },
		{ "ClampMin", "1000.0" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ChunkX_MetaData[] = {
		{ "Category", "MountainGen|Chunk" },
		{ "ClampMax", "512" },
		{ "ClampMin", "8" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// ============================================================\n// 2) Chunk / sampling\n// ============================================================\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "2) Chunk / sampling" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ChunkY_MetaData[] = {
		{ "Category", "MountainGen|Chunk" },
		{ "ClampMax", "512" },
		{ "ClampMin", "8" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ChunkZ_MetaData[] = {
		{ "Category", "MountainGen|Chunk" },
		{ "ClampMax", "512" },
		{ "ClampMin", "8" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_VoxelSizeCm_MetaData[] = {
		{ "Category", "MountainGen|Chunk" },
		{ "ClampMin", "1.0" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_IsoLevel_MetaData[] = {
		{ "Category", "MountainGen|Meshing" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// ============================================================\n// 3) Meshing\n// ============================================================\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "3) Meshing" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_bCreateCollision_MetaData[] = {
		{ "Category", "MountainGen|Meshing" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Difficulty_MetaData[] = {
		{ "Category", "MountainGen|AutoTune" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// ============================================================\n// 4) Difficulty / AutoTune\n// ============================================================\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "4) Difficulty / AutoTune" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_bAutoTune_MetaData[] = {
		{ "Category", "MountainGen|AutoTune" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// true = Seed\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xc4\xb6\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xc4\xbb\xef\xbf\xbd + \xef\xbf\xbd\xc7\xb5\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xdd\xba\xef\xbf\xbd + Seed \xc8\xae\xef\xbf\xbd\xef\xbf\xbd\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "true = Seed\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xc4\xb6\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xc4\xbb\xef\xbf\xbd + \xef\xbf\xbd\xc7\xb5\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xdd\xba\xef\xbf\xbd + Seed \xc8\xae\xef\xbf\xbd\xef\xbf\xbd" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_AutoTuneMaxIters_MetaData[] = {
		{ "Category", "MountainGen|AutoTune" },
		{ "ClampMax", "20" },
		{ "ClampMin", "1" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xef\xbf\xbd\xc7\xb5\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xdd\xba\xef\xbf\xbd \xc8\xbd\xef\xbf\xbd\xef\xbf\xbd\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xef\xbf\xbd\xc7\xb5\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xdd\xba\xef\xbf\xbd \xc8\xbd\xef\xbf\xbd\xef\xbf\xbd" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_SeedSearchTries_MetaData[] = {
		{ "Category", "MountainGen|AutoTune" },
		{ "ClampMax", "2000" },
		{ "ClampMin", "1" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Seed \xef\xbf\xbd\xc4\xba\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd \xc3\xa3\xef\xbf\xbd\xc6\xba\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Seed \xef\xbf\xbd\xc4\xba\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd \xc3\xa3\xef\xbf\xbd\xc6\xba\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Targets_MetaData[] = {
		{ "Category", "MountainGen|AutoTune" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_SteepDotThreshold_MetaData[] = {
		{ "Category", "MountainGen|AutoTune" },
		{ "ClampMax", "1.0" },
		{ "ClampMin", "0.0" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xef\xbf\xbd\xde\xb0\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xef\xbf\xbd\xde\xb0\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_EnvelopeEdgeFadeCm_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Envelope" },
		{ "ClampMin", "100.0" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// ---------------- Envelope shaping ----------------\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "---------------- Envelope shaping ----------------" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_EnvelopeEdgeCutCm_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Envelope" },
		{ "ClampMin", "0.0" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_BaseField3DStrengthCm_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Field3D" },
		{ "ClampMin", "0.0" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// ---------------- Base 3D field ----------------\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "---------------- Base 3D field ----------------" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_BaseField3DScaleCm_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Field3D" },
		{ "ClampMin", "100.0" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_BaseField3DOctaves_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Field3D" },
		{ "ClampMax", "8" },
		{ "ClampMin", "1" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_BaseField3DRidgedPower_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Field3D" },
		{ "ClampMax", "4.0" },
		{ "ClampMin", "1.0" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_DetailScaleCm_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Detail" },
		{ "ClampMin", "300.0" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// ---------------- Detail ----------------\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "---------------- Detail ----------------" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_GravityStrength_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Gravity" },
		{ "ClampMin", "0.01" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// ---------------- Gravity shaping ----------------\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "---------------- Gravity shaping ----------------" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_GravityScale_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Gravity" },
		{ "ClampMin", "0.1" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_WarpPatchCm_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Warp" },
		{ "ClampMin", "1.0" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// ---------------- Warp ----------------\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "---------------- Warp ----------------" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_WarpAmpCm_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Warp" },
		{ "ClampMin", "0.0" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_WarpStrength_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Warp" },
		{ "ClampMin", "0.0" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OverhangScaleCm_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Overhang" },
		{ "ClampMin", "1.0" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// ---------------- Overhang ----------------\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "---------------- Overhang ----------------" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_VolumeStrength_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Overhang" },
		{ "ClampMin", "0.0" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OverhangFadeCm_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Overhang" },
		{ "ClampMin", "0.0" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OverhangBias_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Overhang" },
		{ "ClampMax", "1.0" },
		{ "ClampMin", "0.0" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OverhangDepthCm_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Overhang" },
		{ "ClampMin", "0.0" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveStrength_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Cave" },
		{ "ClampMin", "0.0" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// ---------------- Caves ----------------\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "---------------- Caves ----------------" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveScaleCm_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Cave" },
		{ "ClampMin", "1.0" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveMinHeightCm_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Cave" },
		{ "ClampMin", "0.0" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Cave height band\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Cave height band" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveMaxHeightCm_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Cave" },
		{ "ClampMin", "0.0" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveThreshold_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Cave" },
		{ "ClampMax", "1.0" },
		{ "ClampMin", "0.0" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveBand_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Cave" },
		{ "ClampMax", "1.0" },
		{ "ClampMin", "0.0" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveDepthCm_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Cave" },
		{ "ClampMin", "0.0" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveNearSurfaceCm_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Cave" },
		{ "ClampMin", "0.0" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_bOverhangBiasIncreaseWhenValueIncreases_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Safety" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// ---------------- switches ----------------\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "---------------- switches ----------------" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_bCaveHeightsAreAbsoluteWorldZ_MetaData[] = {
		{ "AdvancedDisplay", "" },
		{ "Category", "MountainGen|Advanced|Safety" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
#endif // WITH_METADATA

// ********** Begin ScriptStruct FMountainGenSettings constinit property declarations **************
	static const UECodeGen_Private::FIntPropertyParams NewProp_Seed;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_BaseHeightCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_HeightAmpCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_EnvelopeRadiusCm;
	static const UECodeGen_Private::FIntPropertyParams NewProp_ChunkX;
	static const UECodeGen_Private::FIntPropertyParams NewProp_ChunkY;
	static const UECodeGen_Private::FIntPropertyParams NewProp_ChunkZ;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_VoxelSizeCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_IsoLevel;
	static void NewProp_bCreateCollision_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_bCreateCollision;
	static const UECodeGen_Private::FBytePropertyParams NewProp_Difficulty_Underlying;
	static const UECodeGen_Private::FEnumPropertyParams NewProp_Difficulty;
	static void NewProp_bAutoTune_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_bAutoTune;
	static const UECodeGen_Private::FIntPropertyParams NewProp_AutoTuneMaxIters;
	static const UECodeGen_Private::FIntPropertyParams NewProp_SeedSearchTries;
	static const UECodeGen_Private::FStructPropertyParams NewProp_Targets;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_SteepDotThreshold;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_EnvelopeEdgeFadeCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_EnvelopeEdgeCutCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_BaseField3DStrengthCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_BaseField3DScaleCm;
	static const UECodeGen_Private::FIntPropertyParams NewProp_BaseField3DOctaves;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_BaseField3DRidgedPower;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_DetailScaleCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_GravityStrength;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_GravityScale;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_WarpPatchCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_WarpAmpCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_WarpStrength;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_OverhangScaleCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_VolumeStrength;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_OverhangFadeCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_OverhangBias;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_OverhangDepthCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_CaveStrength;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_CaveScaleCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_CaveMinHeightCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_CaveMaxHeightCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_CaveThreshold;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_CaveBand;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_CaveDepthCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_CaveNearSurfaceCm;
	static void NewProp_bOverhangBiasIncreaseWhenValueIncreases_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_bOverhangBiasIncreaseWhenValueIncreases;
	static void NewProp_bCaveHeightsAreAbsoluteWorldZ_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_bCaveHeightsAreAbsoluteWorldZ;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
// ********** End ScriptStruct FMountainGenSettings constinit property declarations ****************
	static void* NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FMountainGenSettings>();
	}
	static const UECodeGen_Private::FStructParams StructParams;
}; // struct Z_Construct_UScriptStruct_FMountainGenSettings_Statics
static FStructRegistrationInfo Z_Registration_Info_UScriptStruct_FMountainGenSettings;
class UScriptStruct* FMountainGenSettings::StaticStruct()
{
	if (!Z_Registration_Info_UScriptStruct_FMountainGenSettings.OuterSingleton)
	{
		Z_Registration_Info_UScriptStruct_FMountainGenSettings.OuterSingleton = GetStaticStruct(Z_Construct_UScriptStruct_FMountainGenSettings, (UObject*)Z_Construct_UPackage__Script_MountainGen(), TEXT("MountainGenSettings"));
	}
	return Z_Registration_Info_UScriptStruct_FMountainGenSettings.OuterSingleton;
	}

// ********** Begin ScriptStruct FMountainGenSettings Property Definitions *************************
const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_Seed = { "Seed", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, Seed), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Seed_MetaData), NewProp_Seed_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_BaseHeightCm = { "BaseHeightCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, BaseHeightCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_BaseHeightCm_MetaData), NewProp_BaseHeightCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_HeightAmpCm = { "HeightAmpCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, HeightAmpCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_HeightAmpCm_MetaData), NewProp_HeightAmpCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_EnvelopeRadiusCm = { "EnvelopeRadiusCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, EnvelopeRadiusCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_EnvelopeRadiusCm_MetaData), NewProp_EnvelopeRadiusCm_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_ChunkX = { "ChunkX", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, ChunkX), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ChunkX_MetaData), NewProp_ChunkX_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_ChunkY = { "ChunkY", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, ChunkY), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ChunkY_MetaData), NewProp_ChunkY_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_ChunkZ = { "ChunkZ", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, ChunkZ), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ChunkZ_MetaData), NewProp_ChunkZ_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_VoxelSizeCm = { "VoxelSizeCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, VoxelSizeCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_VoxelSizeCm_MetaData), NewProp_VoxelSizeCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_IsoLevel = { "IsoLevel", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, IsoLevel), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_IsoLevel_MetaData), NewProp_IsoLevel_MetaData) };
void Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_bCreateCollision_SetBit(void* Obj)
{
	((FMountainGenSettings*)Obj)->bCreateCollision = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_bCreateCollision = { "bCreateCollision", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(FMountainGenSettings), &Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_bCreateCollision_SetBit, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_bCreateCollision_MetaData), NewProp_bCreateCollision_MetaData) };
const UECodeGen_Private::FBytePropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_Difficulty_Underlying = { "UnderlyingType", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Byte, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, nullptr, METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FEnumPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_Difficulty = { "Difficulty", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Enum, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, Difficulty), Z_Construct_UEnum_MountainGen_EMountainGenDifficulty, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Difficulty_MetaData), NewProp_Difficulty_MetaData) }; // 2314225620
void Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_bAutoTune_SetBit(void* Obj)
{
	((FMountainGenSettings*)Obj)->bAutoTune = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_bAutoTune = { "bAutoTune", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(FMountainGenSettings), &Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_bAutoTune_SetBit, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_bAutoTune_MetaData), NewProp_bAutoTune_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_AutoTuneMaxIters = { "AutoTuneMaxIters", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, AutoTuneMaxIters), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_AutoTuneMaxIters_MetaData), NewProp_AutoTuneMaxIters_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_SeedSearchTries = { "SeedSearchTries", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, SeedSearchTries), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_SeedSearchTries_MetaData), NewProp_SeedSearchTries_MetaData) };
const UECodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_Targets = { "Targets", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, Targets), Z_Construct_UScriptStruct_FMGTargets, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Targets_MetaData), NewProp_Targets_MetaData) }; // 1840028818
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_SteepDotThreshold = { "SteepDotThreshold", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, SteepDotThreshold), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_SteepDotThreshold_MetaData), NewProp_SteepDotThreshold_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_EnvelopeEdgeFadeCm = { "EnvelopeEdgeFadeCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, EnvelopeEdgeFadeCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_EnvelopeEdgeFadeCm_MetaData), NewProp_EnvelopeEdgeFadeCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_EnvelopeEdgeCutCm = { "EnvelopeEdgeCutCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, EnvelopeEdgeCutCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_EnvelopeEdgeCutCm_MetaData), NewProp_EnvelopeEdgeCutCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_BaseField3DStrengthCm = { "BaseField3DStrengthCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, BaseField3DStrengthCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_BaseField3DStrengthCm_MetaData), NewProp_BaseField3DStrengthCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_BaseField3DScaleCm = { "BaseField3DScaleCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, BaseField3DScaleCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_BaseField3DScaleCm_MetaData), NewProp_BaseField3DScaleCm_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_BaseField3DOctaves = { "BaseField3DOctaves", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, BaseField3DOctaves), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_BaseField3DOctaves_MetaData), NewProp_BaseField3DOctaves_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_BaseField3DRidgedPower = { "BaseField3DRidgedPower", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, BaseField3DRidgedPower), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_BaseField3DRidgedPower_MetaData), NewProp_BaseField3DRidgedPower_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_DetailScaleCm = { "DetailScaleCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, DetailScaleCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_DetailScaleCm_MetaData), NewProp_DetailScaleCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_GravityStrength = { "GravityStrength", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, GravityStrength), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_GravityStrength_MetaData), NewProp_GravityStrength_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_GravityScale = { "GravityScale", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, GravityScale), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_GravityScale_MetaData), NewProp_GravityScale_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_WarpPatchCm = { "WarpPatchCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, WarpPatchCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_WarpPatchCm_MetaData), NewProp_WarpPatchCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_WarpAmpCm = { "WarpAmpCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, WarpAmpCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_WarpAmpCm_MetaData), NewProp_WarpAmpCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_WarpStrength = { "WarpStrength", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, WarpStrength), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_WarpStrength_MetaData), NewProp_WarpStrength_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_OverhangScaleCm = { "OverhangScaleCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, OverhangScaleCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_OverhangScaleCm_MetaData), NewProp_OverhangScaleCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_VolumeStrength = { "VolumeStrength", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, VolumeStrength), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_VolumeStrength_MetaData), NewProp_VolumeStrength_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_OverhangFadeCm = { "OverhangFadeCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, OverhangFadeCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_OverhangFadeCm_MetaData), NewProp_OverhangFadeCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_OverhangBias = { "OverhangBias", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, OverhangBias), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_OverhangBias_MetaData), NewProp_OverhangBias_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_OverhangDepthCm = { "OverhangDepthCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, OverhangDepthCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_OverhangDepthCm_MetaData), NewProp_OverhangDepthCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_CaveStrength = { "CaveStrength", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, CaveStrength), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CaveStrength_MetaData), NewProp_CaveStrength_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_CaveScaleCm = { "CaveScaleCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, CaveScaleCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CaveScaleCm_MetaData), NewProp_CaveScaleCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_CaveMinHeightCm = { "CaveMinHeightCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, CaveMinHeightCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CaveMinHeightCm_MetaData), NewProp_CaveMinHeightCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_CaveMaxHeightCm = { "CaveMaxHeightCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, CaveMaxHeightCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CaveMaxHeightCm_MetaData), NewProp_CaveMaxHeightCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_CaveThreshold = { "CaveThreshold", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, CaveThreshold), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CaveThreshold_MetaData), NewProp_CaveThreshold_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_CaveBand = { "CaveBand", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, CaveBand), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CaveBand_MetaData), NewProp_CaveBand_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_CaveDepthCm = { "CaveDepthCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, CaveDepthCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CaveDepthCm_MetaData), NewProp_CaveDepthCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_CaveNearSurfaceCm = { "CaveNearSurfaceCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, CaveNearSurfaceCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CaveNearSurfaceCm_MetaData), NewProp_CaveNearSurfaceCm_MetaData) };
void Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_bOverhangBiasIncreaseWhenValueIncreases_SetBit(void* Obj)
{
	((FMountainGenSettings*)Obj)->bOverhangBiasIncreaseWhenValueIncreases = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_bOverhangBiasIncreaseWhenValueIncreases = { "bOverhangBiasIncreaseWhenValueIncreases", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(FMountainGenSettings), &Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_bOverhangBiasIncreaseWhenValueIncreases_SetBit, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_bOverhangBiasIncreaseWhenValueIncreases_MetaData), NewProp_bOverhangBiasIncreaseWhenValueIncreases_MetaData) };
void Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_bCaveHeightsAreAbsoluteWorldZ_SetBit(void* Obj)
{
	((FMountainGenSettings*)Obj)->bCaveHeightsAreAbsoluteWorldZ = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_bCaveHeightsAreAbsoluteWorldZ = { "bCaveHeightsAreAbsoluteWorldZ", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(FMountainGenSettings), &Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_bCaveHeightsAreAbsoluteWorldZ_SetBit, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_bCaveHeightsAreAbsoluteWorldZ_MetaData), NewProp_bCaveHeightsAreAbsoluteWorldZ_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FMountainGenSettings_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_Seed,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_BaseHeightCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_HeightAmpCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_EnvelopeRadiusCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_ChunkX,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_ChunkY,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_ChunkZ,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_VoxelSizeCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_IsoLevel,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_bCreateCollision,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_Difficulty_Underlying,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_Difficulty,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_bAutoTune,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_AutoTuneMaxIters,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_SeedSearchTries,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_Targets,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_SteepDotThreshold,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_EnvelopeEdgeFadeCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_EnvelopeEdgeCutCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_BaseField3DStrengthCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_BaseField3DScaleCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_BaseField3DOctaves,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_BaseField3DRidgedPower,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_DetailScaleCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_GravityStrength,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_GravityScale,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_WarpPatchCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_WarpAmpCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_WarpStrength,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_OverhangScaleCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_VolumeStrength,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_OverhangFadeCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_OverhangBias,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_OverhangDepthCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_CaveStrength,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_CaveScaleCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_CaveMinHeightCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_CaveMaxHeightCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_CaveThreshold,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_CaveBand,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_CaveDepthCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_CaveNearSurfaceCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_bOverhangBiasIncreaseWhenValueIncreases,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_bCaveHeightsAreAbsoluteWorldZ,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FMountainGenSettings_Statics::PropPointers) < 2048);
// ********** End ScriptStruct FMountainGenSettings Property Definitions ***************************
const UECodeGen_Private::FStructParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::StructParams = {
	(UObject* (*)())Z_Construct_UPackage__Script_MountainGen,
	nullptr,
	&NewStructOps,
	"MountainGenSettings",
	Z_Construct_UScriptStruct_FMountainGenSettings_Statics::PropPointers,
	UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FMountainGenSettings_Statics::PropPointers),
	sizeof(FMountainGenSettings),
	alignof(FMountainGenSettings),
	RF_Public|RF_Transient|RF_MarkAsNative,
	EStructFlags(0x00000001),
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FMountainGenSettings_Statics::Struct_MetaDataParams), Z_Construct_UScriptStruct_FMountainGenSettings_Statics::Struct_MetaDataParams)
};
UScriptStruct* Z_Construct_UScriptStruct_FMountainGenSettings()
{
	if (!Z_Registration_Info_UScriptStruct_FMountainGenSettings.InnerSingleton)
	{
		UECodeGen_Private::ConstructUScriptStruct(Z_Registration_Info_UScriptStruct_FMountainGenSettings.InnerSingleton, Z_Construct_UScriptStruct_FMountainGenSettings_Statics::StructParams);
	}
	return CastChecked<UScriptStruct>(Z_Registration_Info_UScriptStruct_FMountainGenSettings.InnerSingleton);
}
// ********** End ScriptStruct FMountainGenSettings ************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_Users_User_Documents_GitHub_CDPFinalProject_Mountain_Plugins_MountainGen_Source_MountainGen_Public_MountainGenSettings_h__Script_MountainGen_Statics
{
	static constexpr FEnumRegisterCompiledInInfo EnumInfo[] = {
		{ EMountainGenDifficulty_StaticEnum, TEXT("EMountainGenDifficulty"), &Z_Registration_Info_UEnum_EMountainGenDifficulty, CONSTRUCT_RELOAD_VERSION_INFO(FEnumReloadVersionInfo, 2314225620U) },
	};
	static constexpr FStructRegisterCompiledInInfo ScriptStructInfo[] = {
		{ FMGTargets::StaticStruct, Z_Construct_UScriptStruct_FMGTargets_Statics::NewStructOps, TEXT("MGTargets"),&Z_Registration_Info_UScriptStruct_FMGTargets, CONSTRUCT_RELOAD_VERSION_INFO(FStructReloadVersionInfo, sizeof(FMGTargets), 1840028818U) },
		{ FMountainGenSettings::StaticStruct, Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewStructOps, TEXT("MountainGenSettings"),&Z_Registration_Info_UScriptStruct_FMountainGenSettings, CONSTRUCT_RELOAD_VERSION_INFO(FStructReloadVersionInfo, sizeof(FMountainGenSettings), 448512426U) },
	};
}; // Z_CompiledInDeferFile_FID_Users_User_Documents_GitHub_CDPFinalProject_Mountain_Plugins_MountainGen_Source_MountainGen_Public_MountainGenSettings_h__Script_MountainGen_Statics 
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_User_Documents_GitHub_CDPFinalProject_Mountain_Plugins_MountainGen_Source_MountainGen_Public_MountainGenSettings_h__Script_MountainGen_488203666{
	TEXT("/Script/MountainGen"),
	nullptr, 0,
	Z_CompiledInDeferFile_FID_Users_User_Documents_GitHub_CDPFinalProject_Mountain_Plugins_MountainGen_Source_MountainGen_Public_MountainGenSettings_h__Script_MountainGen_Statics::ScriptStructInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_User_Documents_GitHub_CDPFinalProject_Mountain_Plugins_MountainGen_Source_MountainGen_Public_MountainGenSettings_h__Script_MountainGen_Statics::ScriptStructInfo),
	Z_CompiledInDeferFile_FID_Users_User_Documents_GitHub_CDPFinalProject_Mountain_Plugins_MountainGen_Source_MountainGen_Public_MountainGenSettings_h__Script_MountainGen_Statics::EnumInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_User_Documents_GitHub_CDPFinalProject_Mountain_Plugins_MountainGen_Source_MountainGen_Public_MountainGenSettings_h__Script_MountainGen_Statics::EnumInfo),
};
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
