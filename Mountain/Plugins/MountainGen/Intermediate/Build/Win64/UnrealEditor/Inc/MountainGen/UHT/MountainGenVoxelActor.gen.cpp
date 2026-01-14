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
		{ "Comment", "// ===== Chunk Size (\xeb\xb3\xb5\xec\x85\x80 \xea\xb0\x9c\xec\x88\x98) =====\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "===== Chunk Size (\xeb\xb3\xb5\xec\x85\x80 \xea\xb0\x9c\xec\x88\x98) =====" },
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
		{ "Category", "MountainGen|Noise" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// ===== Noise Params =====\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "===== Noise Params =====" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_HeightScale_MetaData[] = {
		{ "Category", "MountainGen|Noise" },
		{ "ClampMin", "0.0001" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_HeightAmp_MetaData[] = {
		{ "Category", "MountainGen|Noise" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveScale_MetaData[] = {
		{ "Category", "MountainGen|Noise" },
		{ "ClampMin", "0.0001" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_CaveStrength_MetaData[] = {
		{ "Category", "MountainGen|Noise" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_BaseFloor_MetaData[] = {
		{ "Category", "MountainGen|Noise" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_VoxelMaterial_MetaData[] = {
		{ "Category", "MountainGen|Material" },
		{ "ModuleRelativePath", "Public/MountainGenVoxelActor.h" },
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FObjectPropertyParams NewProp_ProcMesh;
	static const UECodeGen_Private::FIntPropertyParams NewProp_ChunkX;
	static const UECodeGen_Private::FIntPropertyParams NewProp_ChunkY;
	static const UECodeGen_Private::FIntPropertyParams NewProp_ChunkZ;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_VoxelSize;
	static const UECodeGen_Private::FIntPropertyParams NewProp_Seed;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_HeightScale;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_HeightAmp;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_CaveScale;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_CaveStrength;
	static const UECodeGen_Private::FFloatPropertyParams NewProp_BaseFloor;
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
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_HeightScale = { "HeightScale", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, HeightScale), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_HeightScale_MetaData), NewProp_HeightScale_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_HeightAmp = { "HeightAmp", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, HeightAmp), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_HeightAmp_MetaData), NewProp_HeightAmp_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_CaveScale = { "CaveScale", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, CaveScale), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CaveScale_MetaData), NewProp_CaveScale_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_CaveStrength = { "CaveStrength", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, CaveStrength), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_CaveStrength_MetaData), NewProp_CaveStrength_MetaData) };
const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_BaseFloor = { "BaseFloor", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, BaseFloor), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_BaseFloor_MetaData), NewProp_BaseFloor_MetaData) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_VoxelMaterial = { "VoxelMaterial", nullptr, (EPropertyFlags)0x0020080000000005, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenVoxelActor, VoxelMaterial), Z_Construct_UClass_UMaterialInterface_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_VoxelMaterial_MetaData), NewProp_VoxelMaterial_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_AMountainGenVoxelActor_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_ProcMesh,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_ChunkX,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_ChunkY,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_ChunkZ,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_VoxelSize,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_Seed,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_HeightScale,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_HeightAmp,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_CaveScale,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_CaveStrength,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenVoxelActor_Statics::NewProp_BaseFloor,
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
struct Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Mountain_Plugins_MountainGen_Source_MountainGen_Public_MountainGenVoxelActor_h__Script_MountainGen_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_AMountainGenVoxelActor, AMountainGenVoxelActor::StaticClass, TEXT("AMountainGenVoxelActor"), &Z_Registration_Info_UClass_AMountainGenVoxelActor, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AMountainGenVoxelActor), 1173434095U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Mountain_Plugins_MountainGen_Source_MountainGen_Public_MountainGenVoxelActor_h__Script_MountainGen_3516332098(TEXT("/Script/MountainGen"),
	Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Mountain_Plugins_MountainGen_Source_MountainGen_Public_MountainGenVoxelActor_h__Script_MountainGen_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Mountain_Plugins_MountainGen_Source_MountainGen_Public_MountainGenVoxelActor_h__Script_MountainGen_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
