// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "MountainGenChunkTypes.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
static_assert(!UE_WITH_CONSTINIT_UOBJECT, "This generated code can only be compiled with !UE_WITH_CONSTINIT_OBJECT");
void EmptyLinkFunctionForGeneratedCodeMountainGenChunkTypes() {}

// ********** Begin Cross Module References ********************************************************
MOUNTAINGEN_API UScriptStruct* Z_Construct_UScriptStruct_FChunkCoord();
UPackage* Z_Construct_UPackage__Script_MountainGen();
// ********** End Cross Module References **********************************************************

// ********** Begin ScriptStruct FChunkCoord *******************************************************
struct Z_Construct_UScriptStruct_FChunkCoord_Statics
{
	static inline consteval int32 GetStructSize() { return sizeof(FChunkCoord); }
	static inline consteval int16 GetStructAlignment() { return alignof(FChunkCoord); }
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "ModuleRelativePath", "Public/MountainGenChunkTypes.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_X_MetaData[] = {
		{ "Category", "ChunkCoord" },
		{ "ModuleRelativePath", "Public/MountainGenChunkTypes.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Y_MetaData[] = {
		{ "Category", "ChunkCoord" },
		{ "ModuleRelativePath", "Public/MountainGenChunkTypes.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Z_MetaData[] = {
		{ "Category", "ChunkCoord" },
		{ "ModuleRelativePath", "Public/MountainGenChunkTypes.h" },
	};
#endif // WITH_METADATA

// ********** Begin ScriptStruct FChunkCoord constinit property declarations ***********************
	static const UECodeGen_Private::FIntPropertyParams NewProp_X;
	static const UECodeGen_Private::FIntPropertyParams NewProp_Y;
	static const UECodeGen_Private::FIntPropertyParams NewProp_Z;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
// ********** End ScriptStruct FChunkCoord constinit property declarations *************************
	static void* NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FChunkCoord>();
	}
	static const UECodeGen_Private::FStructParams StructParams;
}; // struct Z_Construct_UScriptStruct_FChunkCoord_Statics
static FStructRegistrationInfo Z_Registration_Info_UScriptStruct_FChunkCoord;
class UScriptStruct* FChunkCoord::StaticStruct()
{
	if (!Z_Registration_Info_UScriptStruct_FChunkCoord.OuterSingleton)
	{
		Z_Registration_Info_UScriptStruct_FChunkCoord.OuterSingleton = GetStaticStruct(Z_Construct_UScriptStruct_FChunkCoord, (UObject*)Z_Construct_UPackage__Script_MountainGen(), TEXT("ChunkCoord"));
	}
	return Z_Registration_Info_UScriptStruct_FChunkCoord.OuterSingleton;
	}

// ********** Begin ScriptStruct FChunkCoord Property Definitions **********************************
const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FChunkCoord_Statics::NewProp_X = { "X", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FChunkCoord, X), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_X_MetaData), NewProp_X_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FChunkCoord_Statics::NewProp_Y = { "Y", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FChunkCoord, Y), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Y_MetaData), NewProp_Y_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FChunkCoord_Statics::NewProp_Z = { "Z", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(FChunkCoord, Z), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Z_MetaData), NewProp_Z_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FChunkCoord_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FChunkCoord_Statics::NewProp_X,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FChunkCoord_Statics::NewProp_Y,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FChunkCoord_Statics::NewProp_Z,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FChunkCoord_Statics::PropPointers) < 2048);
// ********** End ScriptStruct FChunkCoord Property Definitions ************************************
const UECodeGen_Private::FStructParams Z_Construct_UScriptStruct_FChunkCoord_Statics::StructParams = {
	(UObject* (*)())Z_Construct_UPackage__Script_MountainGen,
	nullptr,
	&NewStructOps,
	"ChunkCoord",
	Z_Construct_UScriptStruct_FChunkCoord_Statics::PropPointers,
	UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FChunkCoord_Statics::PropPointers),
	sizeof(FChunkCoord),
	alignof(FChunkCoord),
	RF_Public|RF_Transient|RF_MarkAsNative,
	EStructFlags(0x00000001),
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FChunkCoord_Statics::Struct_MetaDataParams), Z_Construct_UScriptStruct_FChunkCoord_Statics::Struct_MetaDataParams)
};
UScriptStruct* Z_Construct_UScriptStruct_FChunkCoord()
{
	if (!Z_Registration_Info_UScriptStruct_FChunkCoord.InnerSingleton)
	{
		UECodeGen_Private::ConstructUScriptStruct(Z_Registration_Info_UScriptStruct_FChunkCoord.InnerSingleton, Z_Construct_UScriptStruct_FChunkCoord_Statics::StructParams);
	}
	return CastChecked<UScriptStruct>(Z_Registration_Info_UScriptStruct_FChunkCoord.InnerSingleton);
}
// ********** End ScriptStruct FChunkCoord *********************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Prototype_Plugins_MountainGen_Source_MountainGen_Public_MountainGenChunkTypes_h__Script_MountainGen_Statics
{
	static constexpr FStructRegisterCompiledInInfo ScriptStructInfo[] = {
		{ FChunkCoord::StaticStruct, Z_Construct_UScriptStruct_FChunkCoord_Statics::NewStructOps, TEXT("ChunkCoord"),&Z_Registration_Info_UScriptStruct_FChunkCoord, CONSTRUCT_RELOAD_VERSION_INFO(FStructReloadVersionInfo, sizeof(FChunkCoord), 811009883U) },
	};
}; // Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Prototype_Plugins_MountainGen_Source_MountainGen_Public_MountainGenChunkTypes_h__Script_MountainGen_Statics 
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Prototype_Plugins_MountainGen_Source_MountainGen_Public_MountainGenChunkTypes_h__Script_MountainGen_1817173256{
	TEXT("/Script/MountainGen"),
	nullptr, 0,
	Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Prototype_Plugins_MountainGen_Source_MountainGen_Public_MountainGenChunkTypes_h__Script_MountainGen_Statics::ScriptStructInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Prototype_Plugins_MountainGen_Source_MountainGen_Public_MountainGenChunkTypes_h__Script_MountainGen_Statics::ScriptStructInfo),
	nullptr, 0,
};
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
