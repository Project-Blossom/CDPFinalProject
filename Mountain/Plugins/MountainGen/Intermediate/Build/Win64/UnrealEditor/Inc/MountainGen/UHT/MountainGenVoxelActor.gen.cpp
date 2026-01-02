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
		{ "Comment", "// =========================\n// Chunk Size (\xeb\xb3\xb5\xec\x85\x80 \xea\xb0\x9c\xec\x88\x98)\n// =========================\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Chunk Size (\xeb\xb3\xb5\xec\x85\x80 \xea\xb0\x9c\xec\x88\x98)" },
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
		{ "ClampMax", "256" },
		{ "ClampMin", "4" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_VoxelSize_MetaData[] = {
		{ "Category", "MountainGen|Chunk" },
		{ "ClampMin", "1.0" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xeb\xb3\xb5\xec\x85\x80 \xed\x81\xac\xea\xb8\xb0(cm)\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xeb\xb3\xb5\xec\x85\x80 \xed\x81\xac\xea\xb8\xb0(cm)" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Seed_MetaData[] = {
		{ "Category", "MountainGen|Density" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// =========================\n// Seed\n// =========================\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Seed" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_WorldFreq_MetaData[] = {
		{ "Category", "MountainGen|Density" },
		{ "ClampMin", "0.000001" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// =========================\n// 3D Density Params \n// =========================\n// \xed\x81\xb0 \xed\x98\x95\xed\x83\x9c(\xec\x82\xb0 \xeb\x8d\xa9\xec\x96\xb4\xeb\xa6\xac) \xec\xa3\xbc\xed\x8c\x8c\xec\x88\x98 (\xec\x9e\x91\xec\x9d\x84\xec\x88\x98\xeb\xa1\x9d \xed\x81\xb0 \xeb\x8d\xa9\xec\x96\xb4\xeb\xa6\xac)\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "3D Density Params\n\n\xed\x81\xb0 \xed\x98\x95\xed\x83\x9c(\xec\x82\xb0 \xeb\x8d\xa9\xec\x96\xb4\xeb\xa6\xac) \xec\xa3\xbc\xed\x8c\x8c\xec\x88\x98 (\xec\x9e\x91\xec\x9d\x84\xec\x88\x98\xeb\xa1\x9d \xed\x81\xb0 \xeb\x8d\xa9\xec\x96\xb4\xeb\xa6\xac)" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_DetailFreq_MetaData[] = {
		{ "Category", "MountainGen|Density" },
		{ "ClampMin", "0.000001" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xeb\x94\x94\xed\x85\x8c\xec\x9d\xbc/\xec\x98\xa4\xeb\xb2\x84\xed\x96\x89 \xec\xa3\xbc\xed\x8c\x8c\xec\x88\x98\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xeb\x94\x94\xed\x85\x8c\xec\x9d\xbc/\xec\x98\xa4\xeb\xb2\x84\xed\x96\x89 \xec\xa3\xbc\xed\x8c\x8c\xec\x88\x98" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveFreq_MetaData[] = {
		{ "Category", "MountainGen|Density" },
		{ "ClampMin", "0.000001" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xeb\x8f\x99\xea\xb5\xb4 \xec\xa3\xbc\xed\x8c\x8c\xec\x88\x98\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xeb\x8f\x99\xea\xb5\xb4 \xec\xa3\xbc\xed\x8c\x8c\xec\x88\x98" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_GroundSlope_MetaData[] = {
		{ "Category", "MountainGen|Density" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xec\x95\x84\xeb\x9e\x98\xec\xaa\xbd\xec\x9d\xb4 solid(-), \xec\x9c\x84\xec\xaa\xbd\xec\x9d\xb4 air(+)\xea\xb0\x80 \xeb\x90\x98\xea\xb2\x8c \xeb\xa7\x8c\xeb\x93\x9c\xeb\x8a\x94 \xe2\x80\x9c\xeb\xb0\x94\xeb\x8b\xa5 \xea\xb2\xbd\xed\x96\xa5\xe2\x80\x9d\n// \xea\xb0\x92\xec\x9d\xb4 \xeb\x84\x88\xeb\xac\xb4 \xed\x81\xac\xeb\xa9\xb4 \xec\x9c\x84\xea\xb0\x80 \xeb\x8b\xa4 \xea\xb3\xb5\xea\xb8\xb0(+), \xeb\x84\x88\xeb\xac\xb4 \xec\x9e\x91\xec\x9c\xbc\xeb\xa9\xb4 \xec\xa0\x84\xeb\xb6\x80 \xeb\xb0\x94\xec\x9c\x84(-)\xea\xb0\x80 \xeb\x90\x98\xea\xb8\xb0 \xec\x89\xac\xec\x9b\x80\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xec\x95\x84\xeb\x9e\x98\xec\xaa\xbd\xec\x9d\xb4 solid(-), \xec\x9c\x84\xec\xaa\xbd\xec\x9d\xb4 air(+)\xea\xb0\x80 \xeb\x90\x98\xea\xb2\x8c \xeb\xa7\x8c\xeb\x93\x9c\xeb\x8a\x94 \xe2\x80\x9c\xeb\xb0\x94\xeb\x8b\xa5 \xea\xb2\xbd\xed\x96\xa5\xe2\x80\x9d\n\xea\xb0\x92\xec\x9d\xb4 \xeb\x84\x88\xeb\xac\xb4 \xed\x81\xac\xeb\xa9\xb4 \xec\x9c\x84\xea\xb0\x80 \xeb\x8b\xa4 \xea\xb3\xb5\xea\xb8\xb0(+), \xeb\x84\x88\xeb\xac\xb4 \xec\x9e\x91\xec\x9c\xbc\xeb\xa9\xb4 \xec\xa0\x84\xeb\xb6\x80 \xeb\xb0\x94\xec\x9c\x84(-)\xea\xb0\x80 \xeb\x90\x98\xea\xb8\xb0 \xec\x89\xac\xec\x9b\x80" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_BaseBias_MetaData[] = {
		{ "Category", "MountainGen|Density" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xec\xa0\x84\xec\xb2\xb4 \xeb\xb0\x80\xeb\x8f\x84 \xeb\xb0\x94\xec\x9d\xb4\xec\x96\xb4\xec\x8a\xa4(\xec\x82\xb0\xec\x9d\xb4 \xeb\x84\x88\xeb\xac\xb4 \xea\xb3\xb5\xea\xb8\xb0/\xeb\xb0\x94\xec\x9c\x84\xeb\xa1\x9c \xec\xb9\x98\xec\x9a\xb0\xec\xb9\x98\xeb\xa9\xb4 \xec\x9d\xb4\xea\xb1\xb8\xeb\xa1\x9c \xec\x9d\xb4\xeb\x8f\x99)\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xec\xa0\x84\xec\xb2\xb4 \xeb\xb0\x80\xeb\x8f\x84 \xeb\xb0\x94\xec\x9d\xb4\xec\x96\xb4\xec\x8a\xa4(\xec\x82\xb0\xec\x9d\xb4 \xeb\x84\x88\xeb\xac\xb4 \xea\xb3\xb5\xea\xb8\xb0/\xeb\xb0\x94\xec\x9c\x84\xeb\xa1\x9c \xec\xb9\x98\xec\x9a\xb0\xec\xb9\x98\xeb\xa9\xb4 \xec\x9d\xb4\xea\xb1\xb8\xeb\xa1\x9c \xec\x9d\xb4\xeb\x8f\x99)" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_OverhangAmp_MetaData[] = {
		{ "Category", "MountainGen|Density" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xec\x98\xa4\xeb\xb2\x84\xed\x96\x89/\xea\xb1\xb0\xec\xb9\xa0\xea\xb8\xb0 \xea\xb0\x95\xeb\x8f\x84\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xec\x98\xa4\xeb\xb2\x84\xed\x96\x89/\xea\xb1\xb0\xec\xb9\xa0\xea\xb8\xb0 \xea\xb0\x95\xeb\x8f\x84" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveAmp_MetaData[] = {
		{ "Category", "MountainGen|Density" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xeb\x8f\x99\xea\xb5\xb4 \xec\xb9\xb4\xeb\xb9\x99 \xea\xb0\x95\xeb\x8f\x84(\xea\xb3\xb5\xea\xb8\xb0\xec\xaa\xbd\xec\x9c\xbc\xeb\xa1\x9c \xeb\xaf\xb8\xeb\x8a\x94 \xed\x9e\x98)\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xeb\x8f\x99\xea\xb5\xb4 \xec\xb9\xb4\xeb\xb9\x99 \xea\xb0\x95\xeb\x8f\x84(\xea\xb3\xb5\xea\xb8\xb0\xec\xaa\xbd\xec\x9c\xbc\xeb\xa1\x9c \xeb\xaf\xb8\xeb\x8a\x94 \xed\x9e\x98)" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveThreshold_MetaData[] = {
		{ "Category", "MountainGen|Density" },
		{ "ClampMax", "1.0" },
		{ "ClampMin", "0.0" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xeb\x8f\x99\xea\xb5\xb4 \xec\x83\x9d\xec\x84\xb1 \xec\x9e\x84\xea\xb3\x84\xec\xb9\x98(0~1) : \xeb\x86\x92\xec\x9d\x84\xec\x88\x98\xeb\xa1\x9d \xeb\x8f\x99\xea\xb5\xb4\xec\x9d\xb4 \xeb\x93\x9c\xeb\xac\xbc\xec\x96\xb4\xec\xa7\x90\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xeb\x8f\x99\xea\xb5\xb4 \xec\x83\x9d\xec\x84\xb1 \xec\x9e\x84\xea\xb3\x84\xec\xb9\x98(0~1) : \xeb\x86\x92\xec\x9d\x84\xec\x88\x98\xeb\xa1\x9d \xeb\x8f\x99\xea\xb5\xb4\xec\x9d\xb4 \xeb\x93\x9c\xeb\xac\xbc\xec\x96\xb4\xec\xa7\x90" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_VoxelMaterial_MetaData[] = {
		{ "Category", "MountainGen|Material" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// =========================\n// Material\n// =========================\n" },
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
	static const UECodeGen_Private::FIntPropertyParams NewProp_Seed;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_WorldFreq;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_DetailFreq;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_CaveFreq;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_GroundSlope;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_BaseBias;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_OverhangAmp;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_CaveAmp;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_CaveThreshold;
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
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_Seed = { "Seed", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, Seed), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Seed_MetaData), NewProp_Seed_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_WorldFreq = { "WorldFreq", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, WorldFreq), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_WorldFreq_MetaData), NewProp_WorldFreq_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_DetailFreq = { "DetailFreq", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, DetailFreq), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_DetailFreq_MetaData), NewProp_DetailFreq_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_CaveFreq = { "CaveFreq", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, CaveFreq), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CaveFreq_MetaData), NewProp_CaveFreq_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_GroundSlope = { "GroundSlope", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, GroundSlope), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_GroundSlope_MetaData), NewProp_GroundSlope_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_BaseBias = { "BaseBias", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, BaseBias), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_BaseBias_MetaData), NewProp_BaseBias_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_OverhangAmp = { "OverhangAmp", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, OverhangAmp), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_OverhangAmp_MetaData), NewProp_OverhangAmp_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_CaveAmp = { "CaveAmp", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, CaveAmp), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CaveAmp_MetaData), NewProp_CaveAmp_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_CaveThreshold = { "CaveThreshold", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, CaveThreshold), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CaveThreshold_MetaData), NewProp_CaveThreshold_MetaData) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_VoxelMaterial = { "VoxelMaterial", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, VoxelMaterial), Z_Construct_UClass_UMaterialInterface_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_VoxelMaterial_MetaData), NewProp_VoxelMaterial_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_AMountainGenVoxelActor_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_ProcMesh,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_ChunkX,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_ChunkY,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_ChunkZ,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_VoxelSize,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_Seed,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_WorldFreq,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_DetailFreq,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_CaveFreq,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_GroundSlope,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_BaseBias,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_OverhangAmp,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_CaveAmp,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_CaveThreshold,
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
		{ Z_Construct_UClass_AMountainGenVoxelActor, AMountainGenVoxelActor::StaticClass, TEXT("AMountainGenVoxelActor"), &Z_Registration_Info_UClass_AMountainGenVoxelActor, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AMountainGenVoxelActor), 3458091737U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_b1234_Plugins_MountainGen_Source_MountainGen_Public_MountainGenVoxelActor_h__Script_MountainGen_2513255965(TEXT("/Script/MountainGen"),
	Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_b1234_Plugins_MountainGen_Source_MountainGen_Public_MountainGenVoxelActor_h__Script_MountainGen_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_b1234_Plugins_MountainGen_Source_MountainGen_Public_MountainGenVoxelActor_h__Script_MountainGen_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
