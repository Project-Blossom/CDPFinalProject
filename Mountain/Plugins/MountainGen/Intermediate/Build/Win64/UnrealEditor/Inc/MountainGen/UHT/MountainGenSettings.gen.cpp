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
MOUNTAINGEN_API UScriptStruct* Z_Construct_UScriptStruct_FMountainGenSettings();
UPackage* Z_Construct_UPackage__Script_MountainGen();
// ********** End Cross Module References **********************************************************

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
		{ "Category", "MountainGen|Seed" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ChunkX_MetaData[] = {
		{ "Category", "MountainGen|Chunk" },
		{ "ClampMax", "512" },
		{ "ClampMin", "8" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// ===== Chunk =====\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "===== Chunk =====" },
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
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_bCreateCollision_MetaData[] = {
		{ "Category", "MountainGen|Meshing" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_WorldScaleCm_MetaData[] = {
		{ "Category", "MountainGen|Noise" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// ===== Terrain Params =====\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "===== Terrain Params =====" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_DetailScaleCm_MetaData[] = {
		{ "Category", "MountainGen|Noise" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_BaseHeightCm_MetaData[] = {
		{ "Category", "MountainGen|Height" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_HeightAmpCm_MetaData[] = {
		{ "Category", "MountainGen|Height" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_RampLengthCm_MetaData[] = {
		{ "Category", "MountainGen|Ramp" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_RampHeightCm_MetaData[] = {
		{ "Category", "MountainGen|Ramp" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_WarpPatchCm_MetaData[] = {
		{ "Category", "MountainGen|Warp" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Warp\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Warp" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_WarpAmpCm_MetaData[] = {
		{ "Category", "MountainGen|Warp" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_WarpStrength_MetaData[] = {
		{ "Category", "MountainGen|Warp" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_VolumeStrength_MetaData[] = {
		{ "Category", "MountainGen|Volume" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Volume/Overhang\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Volume/Overhang" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OverhangFadeCm_MetaData[] = {
		{ "Category", "MountainGen|Volume" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OverhangBias_MetaData[] = {
		{ "Category", "MountainGen|Volume" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OverhangDepthCm_MetaData[] = {
		{ "Category", "MountainGen|Volume" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveStrength_MetaData[] = {
		{ "Category", "MountainGen|Cave" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Cave\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Cave" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveScaleCm_MetaData[] = {
		{ "Category", "MountainGen|Cave" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveMinHeightCm_MetaData[] = {
		{ "Category", "MountainGen|Cave" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveMaxHeightCm_MetaData[] = {
		{ "Category", "MountainGen|Cave" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveThreshold_MetaData[] = {
		{ "Category", "MountainGen|Cave" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveBand_MetaData[] = {
		{ "Category", "MountainGen|Cave" },
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_GravityStrength_MetaData[] = {
		{ "Category", "MountainGen|Misc" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Misc\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenSettings.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Misc" },
#endif
	};
#endif // WITH_METADATA

// ********** Begin ScriptStruct FMountainGenSettings constinit property declarations **************
	static const UECodeGen_Private::FIntPropertyParams NewProp_Seed;
	static const UECodeGen_Private::FIntPropertyParams NewProp_ChunkX;
	static const UECodeGen_Private::FIntPropertyParams NewProp_ChunkY;
	static const UECodeGen_Private::FIntPropertyParams NewProp_ChunkZ;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_VoxelSizeCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_IsoLevel;
	static void NewProp_bCreateCollision_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_bCreateCollision;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_WorldScaleCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_DetailScaleCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_BaseHeightCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_HeightAmpCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_RampLengthCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_RampHeightCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_WarpPatchCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_WarpAmpCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_WarpStrength;
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
	static const UECodeGen_Private::FFloatPropertyParams NewProp_GravityStrength;
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
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_WorldScaleCm = { "WorldScaleCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, WorldScaleCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_WorldScaleCm_MetaData), NewProp_WorldScaleCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_DetailScaleCm = { "DetailScaleCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, DetailScaleCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_DetailScaleCm_MetaData), NewProp_DetailScaleCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_BaseHeightCm = { "BaseHeightCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, BaseHeightCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_BaseHeightCm_MetaData), NewProp_BaseHeightCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_HeightAmpCm = { "HeightAmpCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, HeightAmpCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_HeightAmpCm_MetaData), NewProp_HeightAmpCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_RampLengthCm = { "RampLengthCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, RampLengthCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_RampLengthCm_MetaData), NewProp_RampLengthCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_RampHeightCm = { "RampHeightCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, RampHeightCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_RampHeightCm_MetaData), NewProp_RampHeightCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_WarpPatchCm = { "WarpPatchCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, WarpPatchCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_WarpPatchCm_MetaData), NewProp_WarpPatchCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_WarpAmpCm = { "WarpAmpCm", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, WarpAmpCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_WarpAmpCm_MetaData), NewProp_WarpAmpCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_WarpStrength = { "WarpStrength", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, WarpStrength), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_WarpStrength_MetaData), NewProp_WarpStrength_MetaData) };
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
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_GravityStrength = { "GravityStrength", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FMountainGenSettings, GravityStrength), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_GravityStrength_MetaData), NewProp_GravityStrength_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FMountainGenSettings_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_Seed,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_ChunkX,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_ChunkY,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_ChunkZ,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_VoxelSizeCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_IsoLevel,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_bCreateCollision,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_WorldScaleCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_DetailScaleCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_BaseHeightCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_HeightAmpCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_RampLengthCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_RampHeightCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_WarpPatchCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_WarpAmpCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_WarpStrength,
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
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewProp_GravityStrength,
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
struct Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Prototype_Plugins_MountainGen_Source_MountainGen_Public_MountainGenSettings_h__Script_MountainGen_Statics
{
	static constexpr FStructRegisterCompiledInInfo ScriptStructInfo[] = {
		{ FMountainGenSettings::StaticStruct, Z_Construct_UScriptStruct_FMountainGenSettings_Statics::NewStructOps, TEXT("MountainGenSettings"),&Z_Registration_Info_UScriptStruct_FMountainGenSettings, CONSTRUCT_RELOAD_VERSION_INFO(FStructReloadVersionInfo, sizeof(FMountainGenSettings), 653649363U) },
	};
}; // Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Prototype_Plugins_MountainGen_Source_MountainGen_Public_MountainGenSettings_h__Script_MountainGen_Statics 
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Prototype_Plugins_MountainGen_Source_MountainGen_Public_MountainGenSettings_h__Script_MountainGen_2957920200{
	TEXT("/Script/MountainGen"),
	nullptr, 0,
	Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Prototype_Plugins_MountainGen_Source_MountainGen_Public_MountainGenSettings_h__Script_MountainGen_Statics::ScriptStructInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Prototype_Plugins_MountainGen_Source_MountainGen_Public_MountainGenSettings_h__Script_MountainGen_Statics::ScriptStructInfo),
	nullptr, 0,
};
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
