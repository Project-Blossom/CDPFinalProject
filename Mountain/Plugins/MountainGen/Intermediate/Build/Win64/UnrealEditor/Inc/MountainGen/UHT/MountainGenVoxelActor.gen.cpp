// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "MountainGenVoxelActor.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void EmptyLinkFunctionForGeneratedCodeMountainGenVoxelActor() {}

// ********** Begin Cross Module References ********************************************************
ENGINE_API UClass* Z_Construct_UClass_AActor();
ENGINE_API UClass* Z_Construct_UClass_UMaterialInterface_NoRegister();
MOUNTAINGEN_API UClass* Z_Construct_UClass_AMountainGenVoxelActor();
MOUNTAINGEN_API UClass* Z_Construct_UClass_AMountainGenVoxelActor_NoRegister();
PROCEDURALMESHCOMPONENT_API UClass* Z_Construct_UClass_UProceduralMeshComponent_NoRegister();
UPackage* Z_Construct_UPackage__Script_MountainGen();
// ********** End Cross Module References **********************************************************

// ********** Begin Class AMountainGenVoxelActor Function Regenerate *******************************
struct Z_Construct_UFunction_AMountainGenVoxelActor_Regenerate_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "MountainGen" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_AMountainGenVoxelActor_Regenerate_Statics::FuncParams = { { (UObject*(*)())Z_Construct_UClass_AMountainGenVoxelActor, nullptr, "Regenerate", nullptr, 0, 0, RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_AMountainGenVoxelActor_Regenerate_Statics::Function_MetaDataParams), Z_Construct_UFunction_AMountainGenVoxelActor_Regenerate_Statics::Function_MetaDataParams)},  };
UFunction* Z_Construct_UFunction_AMountainGenVoxelActor_Regenerate()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_AMountainGenVoxelActor_Regenerate_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(AMountainGenVoxelActor::execRegenerate)
{
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->Regenerate();
	P_NATIVE_END;
}
// ********** End Class AMountainGenVoxelActor Function Regenerate *********************************

// ********** Begin Class AMountainGenVoxelActor Function SetSeed **********************************
struct Z_Construct_UFunction_AMountainGenVoxelActor_SetSeed_Statics
{
	struct MountainGenVoxelActor_eventSetSeed_Parms
	{
		int32 NewSeed;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "MountainGen" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FIntPropertyParams NewProp_NewSeed;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FIntPropertyParams Z_Construct_UFunction_AMountainGenVoxelActor_SetSeed_Statics::NewProp_NewSeed = { "NewSeed", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(MountainGenVoxelActor_eventSetSeed_Parms, NewSeed), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_AMountainGenVoxelActor_SetSeed_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AMountainGenVoxelActor_SetSeed_Statics::NewProp_NewSeed,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_AMountainGenVoxelActor_SetSeed_Statics::PropPointers) < 2048);
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_AMountainGenVoxelActor_SetSeed_Statics::FuncParams = { { (UObject*(*)())Z_Construct_UClass_AMountainGenVoxelActor, nullptr, "SetSeed", Z_Construct_UFunction_AMountainGenVoxelActor_SetSeed_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_AMountainGenVoxelActor_SetSeed_Statics::PropPointers), sizeof(Z_Construct_UFunction_AMountainGenVoxelActor_SetSeed_Statics::MountainGenVoxelActor_eventSetSeed_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_AMountainGenVoxelActor_SetSeed_Statics::Function_MetaDataParams), Z_Construct_UFunction_AMountainGenVoxelActor_SetSeed_Statics::Function_MetaDataParams)},  };
static_assert(sizeof(Z_Construct_UFunction_AMountainGenVoxelActor_SetSeed_Statics::MountainGenVoxelActor_eventSetSeed_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_AMountainGenVoxelActor_SetSeed()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_AMountainGenVoxelActor_SetSeed_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(AMountainGenVoxelActor::execSetSeed)
{
	P_GET_PROPERTY(FIntProperty,Z_Param_NewSeed);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->SetSeed(Z_Param_NewSeed);
	P_NATIVE_END;
}
// ********** End Class AMountainGenVoxelActor Function SetSeed ************************************

// ********** Begin Class AMountainGenVoxelActor ***************************************************
void AMountainGenVoxelActor::StaticRegisterNativesAMountainGenVoxelActor()
{
	UClass* Class = AMountainGenVoxelActor::StaticClass();
	static const FNameNativePtrPair Funcs[] = {
		{ "Regenerate", &AMountainGenVoxelActor::execRegenerate },
		{ "SetSeed", &AMountainGenVoxelActor::execSetSeed },
	};
	FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
}
FClassRegistrationInfo Z_Registration_Info_UClass_AMountainGenVoxelActor;
UClass* AMountainGenVoxelActor::GetPrivateStaticClass()
{
	using TClass = AMountainGenVoxelActor;
	if (!Z_Registration_Info_UClass_AMountainGenVoxelActor.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			StaticPackage(),
			TEXT("MountainGenVoxelActor"),
			Z_Registration_Info_UClass_AMountainGenVoxelActor.InnerSingleton,
			StaticRegisterNativesAMountainGenVoxelActor,
			sizeof(TClass),
			alignof(TClass),
			TClass::StaticClassFlags,
			TClass::StaticClassCastFlags(),
			TClass::StaticConfigName(),
			(UClass::ClassConstructorType)InternalConstructor<TClass>,
			(UClass::ClassVTableHelperCtorCallerType)InternalVTableHelperCtorCaller<TClass>,
			UOBJECT_CPPCLASS_STATICFUNCTIONS_FORCLASS(TClass),
			&TClass::Super::StaticClass,
			&TClass::WithinClass::StaticClass
		);
	}
	return Z_Registration_Info_UClass_AMountainGenVoxelActor.InnerSingleton;
}
UClass* Z_Construct_UClass_AMountainGenVoxelActor_NoRegister()
{
	return AMountainGenVoxelActor::GetPrivateStaticClass();
}
struct Z_Construct_UClass_AMountainGenVoxelActor_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "IncludePath", "MountainGenVoxelActor.h" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ProcMesh_MetaData[] = {
		{ "Category", "Components" },
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ChunkX_MetaData[] = {
		{ "Category", "MountainGen|Chunk" },
		{ "ClampMax", "256" },
		{ "ClampMin", "4" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// =========================\n// Chunk\n// =========================\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Chunk" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ChunkY_MetaData[] = {
		{ "Category", "MountainGen|Chunk" },
		{ "ClampMax", "256" },
		{ "ClampMin", "4" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ChunkZ_MetaData[] = {
		{ "Category", "MountainGen|Chunk" },
		{ "ClampMax", "512" },
		{ "ClampMin", "8" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_VoxelSize_MetaData[] = {
		{ "Category", "MountainGen|Chunk" },
		{ "ClampMin", "1.0" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_IsoLevel_MetaData[] = {
		{ "Category", "MountainGen|Meshing" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Meshing\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Meshing" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Seed_MetaData[] = {
		{ "Category", "MountainGen|Density" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Seed\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Seed" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_WorldScaleCm_MetaData[] = {
		{ "Category", "MountainGen|Density" },
		{ "ClampMin", "100.0" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// =========================\n// Density Params (Generator)\n// =========================\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Density Params (Generator)" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_DetailScaleCm_MetaData[] = {
		{ "Category", "MountainGen|Density" },
		{ "ClampMin", "100.0" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_BaseHeightCm_MetaData[] = {
		{ "Category", "MountainGen|Density|Height" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_HeightAmpCm_MetaData[] = {
		{ "Category", "MountainGen|Density|Height" },
		{ "ClampMin", "1.0" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_RampHeightCm_MetaData[] = {
		{ "Category", "MountainGen|Density|Height" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xeb\x81\x9d\xec\x9c\xbc\xeb\xa1\x9c \xea\xb0\x88\xec\x88\x98\xeb\xa1\x9d \xeb\x86\x92\xea\xb2\x8c (Peak \xeb\x8a\x90\xeb\x82\x8c)\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xeb\x81\x9d\xec\x9c\xbc\xeb\xa1\x9c \xea\xb0\x88\xec\x88\x98\xeb\xa1\x9d \xeb\x86\x92\xea\xb2\x8c (Peak \xeb\x8a\x90\xeb\x82\x8c)" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_RampLengthCm_MetaData[] = {
		{ "Category", "MountainGen|Density|Height" },
		{ "ClampMin", "1.0" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_VolumeStrength_MetaData[] = {
		{ "Category", "MountainGen|Density|Volume" },
		{ "ClampMin", "0.0" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OverhangFadeCm_MetaData[] = {
		{ "Category", "MountainGen|Density|Volume" },
		{ "ClampMin", "1.0" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_WarpPatchCm_MetaData[] = {
		{ "Category", "MountainGen|Density|Warp" },
		{ "ClampMin", "100.0" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Warp\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Warp" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_WarpAmpCm_MetaData[] = {
		{ "Category", "MountainGen|Density|Warp" },
		{ "ClampMin", "0.0" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_WarpStrength_MetaData[] = {
		{ "Category", "MountainGen|Density|Warp" },
		{ "ClampMin", "0.0" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveScaleCm_MetaData[] = {
		{ "Category", "MountainGen|Density|Caves" },
		{ "ClampMin", "100.0" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Caves\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Caves" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveThreshold_MetaData[] = {
		{ "Category", "MountainGen|Density|Caves" },
		{ "ClampMax", "1.0" },
		{ "ClampMin", "-1.0" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveStrength_MetaData[] = {
		{ "Category", "MountainGen|Density|Caves" },
		{ "ClampMin", "0.0" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveBand_MetaData[] = {
		{ "Category", "MountainGen|Density|Caves" },
		{ "ClampMin", "0.01" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_bRemoveIslands_MetaData[] = {
		{ "Category", "MountainGen|Post" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// =========================\n// Postprocess (Mask)\n// =========================\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Postprocess (Mask)" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_GroundBandZ_MetaData[] = {
		{ "Category", "MountainGen|Post" },
		{ "ClampMax", "16" },
		{ "ClampMin", "1" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xeb\xb0\x94\xeb\x8b\xa5 z=0~GroundBandZ-1 \xec\x9d\x84 \"\xeb\x95\x85\"\xec\x9c\xbc\xeb\xa1\x9c \xeb\xb3\xb4\xea\xb3\xa0 \xec\x97\xb0\xea\xb2\xb0\xeb\x90\x9c solid\xeb\xa7\x8c \xec\x9c\xa0\xec\xa7\x80\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xeb\xb0\x94\xeb\x8b\xa5 z=0~GroundBandZ-1 \xec\x9d\x84 \"\xeb\x95\x85\"\xec\x9c\xbc\xeb\xa1\x9c \xeb\xb3\xb4\xea\xb3\xa0 \xec\x97\xb0\xea\xb2\xb0\xeb\x90\x9c solid\xeb\xa7\x8c \xec\x9c\xa0\xec\xa7\x80" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_bUseClosing_MetaData[] = {
		{ "Category", "MountainGen|Post" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ClosingDilateIters_MetaData[] = {
		{ "Category", "MountainGen|Post" },
		{ "ClampMax", "8" },
		{ "ClampMin", "0" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ClosingErodeIters_MetaData[] = {
		{ "Category", "MountainGen|Post" },
		{ "ClampMax", "8" },
		{ "ClampMin", "0" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_SoftPushCm_MetaData[] = {
		{ "Category", "MountainGen|Post" },
		{ "ClampMin", "0.0" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xe2\x98\x85 \xea\xb3\x84\xeb\x8b\xa8 \xeb\xb0\xa9\xec\xa7\x80 \xed\x95\xb5\xec\x8b\xac: \xeb\xa7\x88\xec\x8a\xa4\xed\x81\xac \xea\xb2\xb0\xea\xb3\xbc\xeb\xa5\xbc density\xec\x97\x90 \"\xec\x82\xb4\xec\xa7\x9d\"\xeb\xa7\x8c \xeb\xb0\x98\xec\x98\x81\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xe2\x98\x85 \xea\xb3\x84\xeb\x8b\xa8 \xeb\xb0\xa9\xec\xa7\x80 \xed\x95\xb5\xec\x8b\xac: \xeb\xa7\x88\xec\x8a\xa4\xed\x81\xac \xea\xb2\xb0\xea\xb3\xbc\xeb\xa5\xbc density\xec\x97\x90 \"\xec\x82\xb4\xec\xa7\x9d\"\xeb\xa7\x8c \xeb\xb0\x98\xec\x98\x81" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_VoxelMaterial_MetaData[] = {
		{ "Category", "MountainGen|Material" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Material\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Material" },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FObjectPropertyParams NewProp_ProcMesh;
	static const UECodeGen_Private::FIntPropertyParams NewProp_ChunkX;
	static const UECodeGen_Private::FIntPropertyParams NewProp_ChunkY;
	static const UECodeGen_Private::FIntPropertyParams NewProp_ChunkZ;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_VoxelSize;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_IsoLevel;
	static const UECodeGen_Private::FIntPropertyParams NewProp_Seed;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_WorldScaleCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_DetailScaleCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_BaseHeightCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_HeightAmpCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_RampHeightCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_RampLengthCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_VolumeStrength;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_OverhangFadeCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_WarpPatchCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_WarpAmpCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_WarpStrength;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_CaveScaleCm;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_CaveThreshold;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_CaveStrength;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_CaveBand;
	static void NewProp_bRemoveIslands_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_bRemoveIslands;
	static const UECodeGen_Private::FIntPropertyParams NewProp_GroundBandZ;
	static void NewProp_bUseClosing_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_bUseClosing;
	static const UECodeGen_Private::FIntPropertyParams NewProp_ClosingDilateIters;
	static const UECodeGen_Private::FIntPropertyParams NewProp_ClosingErodeIters;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_SoftPushCm;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_VoxelMaterial;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FClassFunctionLinkInfo FuncInfo[] = {
		{ &Z_Construct_UFunction_AMountainGenVoxelActor_Regenerate, "Regenerate" }, // 672175376
		{ &Z_Construct_UFunction_AMountainGenVoxelActor_SetSeed, "SetSeed" }, // 893884356
	};
	static_assert(UE_ARRAY_COUNT(FuncInfo) < 2048);
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AMountainGenVoxelActor>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_ProcMesh = { "ProcMesh", nullptr, (EPropertyFlags)0x00200800000a0009, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, ProcMesh), Z_Construct_UClass_UProceduralMeshComponent_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ProcMesh_MetaData), NewProp_ProcMesh_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_ChunkX = { "ChunkX", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, ChunkX), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ChunkX_MetaData), NewProp_ChunkX_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_ChunkY = { "ChunkY", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, ChunkY), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ChunkY_MetaData), NewProp_ChunkY_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_ChunkZ = { "ChunkZ", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, ChunkZ), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ChunkZ_MetaData), NewProp_ChunkZ_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_VoxelSize = { "VoxelSize", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, VoxelSize), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_VoxelSize_MetaData), NewProp_VoxelSize_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_IsoLevel = { "IsoLevel", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, IsoLevel), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_IsoLevel_MetaData), NewProp_IsoLevel_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_Seed = { "Seed", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, Seed), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Seed_MetaData), NewProp_Seed_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_WorldScaleCm = { "WorldScaleCm", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, WorldScaleCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_WorldScaleCm_MetaData), NewProp_WorldScaleCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_DetailScaleCm = { "DetailScaleCm", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, DetailScaleCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_DetailScaleCm_MetaData), NewProp_DetailScaleCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_BaseHeightCm = { "BaseHeightCm", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, BaseHeightCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_BaseHeightCm_MetaData), NewProp_BaseHeightCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_HeightAmpCm = { "HeightAmpCm", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, HeightAmpCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_HeightAmpCm_MetaData), NewProp_HeightAmpCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_RampHeightCm = { "RampHeightCm", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, RampHeightCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_RampHeightCm_MetaData), NewProp_RampHeightCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_RampLengthCm = { "RampLengthCm", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, RampLengthCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_RampLengthCm_MetaData), NewProp_RampLengthCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_VolumeStrength = { "VolumeStrength", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, VolumeStrength), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_VolumeStrength_MetaData), NewProp_VolumeStrength_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_OverhangFadeCm = { "OverhangFadeCm", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, OverhangFadeCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_OverhangFadeCm_MetaData), NewProp_OverhangFadeCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_WarpPatchCm = { "WarpPatchCm", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, WarpPatchCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_WarpPatchCm_MetaData), NewProp_WarpPatchCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_WarpAmpCm = { "WarpAmpCm", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, WarpAmpCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_WarpAmpCm_MetaData), NewProp_WarpAmpCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_WarpStrength = { "WarpStrength", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, WarpStrength), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_WarpStrength_MetaData), NewProp_WarpStrength_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_CaveScaleCm = { "CaveScaleCm", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, CaveScaleCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CaveScaleCm_MetaData), NewProp_CaveScaleCm_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_CaveThreshold = { "CaveThreshold", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, CaveThreshold), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CaveThreshold_MetaData), NewProp_CaveThreshold_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_CaveStrength = { "CaveStrength", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, CaveStrength), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CaveStrength_MetaData), NewProp_CaveStrength_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_CaveBand = { "CaveBand", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, CaveBand), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CaveBand_MetaData), NewProp_CaveBand_MetaData) };
void Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_bRemoveIslands_SetBit(void* Obj)
{
	((AMountainGenVoxelActor*)Obj)->bRemoveIslands = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_bRemoveIslands = { "bRemoveIslands", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(AMountainGenVoxelActor), &Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_bRemoveIslands_SetBit, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_bRemoveIslands_MetaData), NewProp_bRemoveIslands_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_GroundBandZ = { "GroundBandZ", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, GroundBandZ), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_GroundBandZ_MetaData), NewProp_GroundBandZ_MetaData) };
void Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_bUseClosing_SetBit(void* Obj)
{
	((AMountainGenVoxelActor*)Obj)->bUseClosing = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_bUseClosing = { "bUseClosing", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(AMountainGenVoxelActor), &Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_bUseClosing_SetBit, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_bUseClosing_MetaData), NewProp_bUseClosing_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_ClosingDilateIters = { "ClosingDilateIters", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, ClosingDilateIters), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ClosingDilateIters_MetaData), NewProp_ClosingDilateIters_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_ClosingErodeIters = { "ClosingErodeIters", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, ClosingErodeIters), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ClosingErodeIters_MetaData), NewProp_ClosingErodeIters_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_SoftPushCm = { "SoftPushCm", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, SoftPushCm), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_SoftPushCm_MetaData), NewProp_SoftPushCm_MetaData) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_VoxelMaterial = { "VoxelMaterial", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, VoxelMaterial), Z_Construct_UClass_UMaterialInterface_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_VoxelMaterial_MetaData), NewProp_VoxelMaterial_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_AMountainGenVoxelActor_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_ProcMesh,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_ChunkX,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_ChunkY,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_ChunkZ,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_VoxelSize,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_IsoLevel,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_Seed,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_WorldScaleCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_DetailScaleCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_BaseHeightCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_HeightAmpCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_RampHeightCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_RampLengthCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_VolumeStrength,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_OverhangFadeCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_WarpPatchCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_WarpAmpCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_WarpStrength,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_CaveScaleCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_CaveThreshold,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_CaveStrength,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_CaveBand,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_bRemoveIslands,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_GroundBandZ,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_bUseClosing,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_ClosingDilateIters,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_ClosingErodeIters,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_SoftPushCm,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_VoxelMaterial,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AMountainGenVoxelActor_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_AMountainGenVoxelActor_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_AActor,
	(UObject* (*)())Z_Construct_UPackage__Script_MountainGen,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AMountainGenVoxelActor_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::ClassParams = {
	&AMountainGenVoxelActor::StaticClass,
	"Engine",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	FuncInfo,
	Z_Construct_UClass_AMountainGenVoxelActor_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	UE_ARRAY_COUNT(FuncInfo),
	UE_ARRAY_COUNT(Z_Construct_UClass_AMountainGenVoxelActor_Statics::PropPointers),
	0,
	0x009001A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AMountainGenVoxelActor_Statics::Class_MetaDataParams), Z_Construct_UClass_AMountainGenVoxelActor_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_AMountainGenVoxelActor()
{
	if (!Z_Registration_Info_UClass_AMountainGenVoxelActor.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_AMountainGenVoxelActor.OuterSingleton, Z_Construct_UClass_AMountainGenVoxelActor_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_AMountainGenVoxelActor.OuterSingleton;
}
DEFINE_VTABLE_PTR_HELPER_CTOR(AMountainGenVoxelActor);
AMountainGenVoxelActor::~AMountainGenVoxelActor() {}
// ********** End Class AMountainGenVoxelActor *****************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_b1234_Plugins_MountainGen_Source_MountainGen_Public_MountainGenVoxelActor_h__Script_MountainGen_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_AMountainGenVoxelActor, AMountainGenVoxelActor::StaticClass, TEXT("AMountainGenVoxelActor"), &Z_Registration_Info_UClass_AMountainGenVoxelActor, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AMountainGenVoxelActor), 2725394020U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_b1234_Plugins_MountainGen_Source_MountainGen_Public_MountainGenVoxelActor_h__Script_MountainGen_2262555333(TEXT("/Script/MountainGen"),
	Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_b1234_Plugins_MountainGen_Source_MountainGen_Public_MountainGenVoxelActor_h__Script_MountainGen_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_b1234_Plugins_MountainGen_Source_MountainGen_Public_MountainGenVoxelActor_h__Script_MountainGen_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
