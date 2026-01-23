// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "MountainGenWorldActor.h"
#include "MountainGenSettings.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
static_assert(!UE_WITH_CONSTINIT_UOBJECT, "This generated code can only be compiled with !UE_WITH_CONSTINIT_OBJECT");
void EmptyLinkFunctionForGeneratedCodeMountainGenWorldActor() {}

// ********** Begin Cross Module References ********************************************************
ENGINE_API UClass* Z_Construct_UClass_AActor();
ENGINE_API UClass* Z_Construct_UClass_UMaterialInterface_NoRegister();
MOUNTAINGEN_API UClass* Z_Construct_UClass_AMountainGenWorldActor();
MOUNTAINGEN_API UClass* Z_Construct_UClass_AMountainGenWorldActor_NoRegister();
MOUNTAINGEN_API UScriptStruct* Z_Construct_UScriptStruct_FMountainGenSettings();
PROCEDURALMESHCOMPONENT_API UClass* Z_Construct_UClass_UProceduralMeshComponent_NoRegister();
UPackage* Z_Construct_UPackage__Script_MountainGen();
// ********** End Cross Module References **********************************************************

// ********** Begin Class AMountainGenWorldActor Function RandomizeSeed ****************************
struct Z_Construct_UFunction_AMountainGenWorldActor_RandomizeSeed_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "MountainGen" },
		{ "ModuleRelativePath", "Public/MountainGenWorldActor.h" },
	};
#endif // WITH_METADATA

// ********** Begin Function RandomizeSeed constinit property declarations *************************
// ********** End Function RandomizeSeed constinit property declarations ***************************
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_AMountainGenWorldActor_RandomizeSeed_Statics::FuncParams = { { (UObject*(*)())Z_Construct_UClass_AMountainGenWorldActor, nullptr, "RandomizeSeed", 	nullptr, 
	0, 
0,
RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_AMountainGenWorldActor_RandomizeSeed_Statics::Function_MetaDataParams), Z_Construct_UFunction_AMountainGenWorldActor_RandomizeSeed_Statics::Function_MetaDataParams)},  };
UFunction* Z_Construct_UFunction_AMountainGenWorldActor_RandomizeSeed()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_AMountainGenWorldActor_RandomizeSeed_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(AMountainGenWorldActor::execRandomizeSeed)
{
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->RandomizeSeed();
	P_NATIVE_END;
}
// ********** End Class AMountainGenWorldActor Function RandomizeSeed ******************************

// ********** Begin Class AMountainGenWorldActor Function Regenerate *******************************
struct Z_Construct_UFunction_AMountainGenWorldActor_Regenerate_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "MountainGen" },
		{ "ModuleRelativePath", "Public/MountainGenWorldActor.h" },
	};
#endif // WITH_METADATA

// ********** Begin Function Regenerate constinit property declarations ****************************
// ********** End Function Regenerate constinit property declarations ******************************
	static const UECodeGen_Private::FFunctionParams FuncParams;
};
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_AMountainGenWorldActor_Regenerate_Statics::FuncParams = { { (UObject*(*)())Z_Construct_UClass_AMountainGenWorldActor, nullptr, "Regenerate", 	nullptr, 
	0, 
0,
RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_AMountainGenWorldActor_Regenerate_Statics::Function_MetaDataParams), Z_Construct_UFunction_AMountainGenWorldActor_Regenerate_Statics::Function_MetaDataParams)},  };
UFunction* Z_Construct_UFunction_AMountainGenWorldActor_Regenerate()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_AMountainGenWorldActor_Regenerate_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(AMountainGenWorldActor::execRegenerate)
{
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->Regenerate();
	P_NATIVE_END;
}
// ********** End Class AMountainGenWorldActor Function Regenerate *********************************

// ********** Begin Class AMountainGenWorldActor Function SetSeed **********************************
struct Z_Construct_UFunction_AMountainGenWorldActor_SetSeed_Statics
{
	struct MountainGenWorldActor_eventSetSeed_Parms
	{
		int32 NewSeed;
	};
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[] = {
		{ "Category", "MountainGen" },
		{ "ModuleRelativePath", "Public/MountainGenWorldActor.h" },
	};
#endif // WITH_METADATA

// ********** Begin Function SetSeed constinit property declarations *******************************
	static const UECodeGen_Private::FIntPropertyParams NewProp_NewSeed;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
// ********** End Function SetSeed constinit property declarations *********************************
	static const UECodeGen_Private::FFunctionParams FuncParams;
};

// ********** Begin Function SetSeed Property Definitions ******************************************
const UECodeGen_Private::FIntPropertyParams Z_Construct_UFunction_AMountainGenWorldActor_SetSeed_Statics::NewProp_NewSeed = { "NewSeed", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(MountainGenWorldActor_eventSetSeed_Parms, NewSeed), METADATA_PARAMS(0, nullptr) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_AMountainGenWorldActor_SetSeed_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AMountainGenWorldActor_SetSeed_Statics::NewProp_NewSeed,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_AMountainGenWorldActor_SetSeed_Statics::PropPointers) < 2048);
// ********** End Function SetSeed Property Definitions ********************************************
const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_AMountainGenWorldActor_SetSeed_Statics::FuncParams = { { (UObject*(*)())Z_Construct_UClass_AMountainGenWorldActor, nullptr, "SetSeed", 	Z_Construct_UFunction_AMountainGenWorldActor_SetSeed_Statics::PropPointers, 
	UE_ARRAY_COUNT(Z_Construct_UFunction_AMountainGenWorldActor_SetSeed_Statics::PropPointers), 
sizeof(Z_Construct_UFunction_AMountainGenWorldActor_SetSeed_Statics::MountainGenWorldActor_eventSetSeed_Parms),
RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_AMountainGenWorldActor_SetSeed_Statics::Function_MetaDataParams), Z_Construct_UFunction_AMountainGenWorldActor_SetSeed_Statics::Function_MetaDataParams)},  };
static_assert(sizeof(Z_Construct_UFunction_AMountainGenWorldActor_SetSeed_Statics::MountainGenWorldActor_eventSetSeed_Parms) < MAX_uint16);
UFunction* Z_Construct_UFunction_AMountainGenWorldActor_SetSeed()
{
	static UFunction* ReturnFunction = nullptr;
	if (!ReturnFunction)
	{
		UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_AMountainGenWorldActor_SetSeed_Statics::FuncParams);
	}
	return ReturnFunction;
}
DEFINE_FUNCTION(AMountainGenWorldActor::execSetSeed)
{
	P_GET_PROPERTY(FIntProperty,Z_Param_NewSeed);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS->SetSeed(Z_Param_NewSeed);
	P_NATIVE_END;
}
// ********** End Class AMountainGenWorldActor Function SetSeed ************************************

// ********** Begin Class AMountainGenWorldActor ***************************************************
FClassRegistrationInfo Z_Registration_Info_UClass_AMountainGenWorldActor;
UClass* AMountainGenWorldActor::GetPrivateStaticClass()
{
	using TClass = AMountainGenWorldActor;
	if (!Z_Registration_Info_UClass_AMountainGenWorldActor.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			TClass::StaticPackage(),
			TEXT("MountainGenWorldActor"),
			Z_Registration_Info_UClass_AMountainGenWorldActor.InnerSingleton,
			StaticRegisterNativesAMountainGenWorldActor,
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
	return Z_Registration_Info_UClass_AMountainGenWorldActor.InnerSingleton;
}
UClass* Z_Construct_UClass_AMountainGenWorldActor_NoRegister()
{
	return AMountainGenWorldActor::GetPrivateStaticClass();
}
struct Z_Construct_UClass_AMountainGenWorldActor_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "IncludePath", "MountainGenWorldActor.h" },
		{ "ModuleRelativePath", "Public/MountainGenWorldActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ProcMesh_MetaData[] = {
		{ "Category", "MountainGen" },
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "Public/MountainGenWorldActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_VoxelMaterial_MetaData[] = {
		{ "Category", "MountainGen|Mesh" },
		{ "ModuleRelativePath", "Public/MountainGenWorldActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Settings_MetaData[] = {
		{ "Category", "MountainGen" },
		{ "ModuleRelativePath", "Public/MountainGenWorldActor.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_bEnableRandomSeedKey_MetaData[] = {
		{ "Category", "MountainGen|Runtime" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xef\xbf\xbd\xef\xbf\xbd\xc5\xb8\xef\xbf\xbd\xd3\xbf\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd 1\xef\xbf\xbd\xef\xbf\xbd \xc5\xb0\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xc3\xb5\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\n" },
#endif
		{ "ModuleRelativePath", "Public/MountainGenWorldActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xef\xbf\xbd\xef\xbf\xbd\xc5\xb8\xef\xbf\xbd\xd3\xbf\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd 1\xef\xbf\xbd\xef\xbf\xbd \xc5\xb0\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xc3\xb5\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
#endif
	};
#endif // WITH_METADATA

// ********** Begin Class AMountainGenWorldActor constinit property declarations *******************
	static const UECodeGen_Private::FObjectPropertyParams NewProp_ProcMesh;
	static const UECodeGen_Private::FObjectPropertyParams NewProp_VoxelMaterial;
	static const UECodeGen_Private::FStructPropertyParams NewProp_Settings;
	static void NewProp_bEnableRandomSeedKey_SetBit(void* Obj);
	static const UECodeGen_Private::FBoolPropertyParams NewProp_bEnableRandomSeedKey;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
// ********** End Class AMountainGenWorldActor constinit property declarations *********************
	static constexpr UE::CodeGen::FClassNativeFunction Funcs[] = {
		{ .NameUTF8 = UTF8TEXT("RandomizeSeed"), .Pointer = &AMountainGenWorldActor::execRandomizeSeed },
		{ .NameUTF8 = UTF8TEXT("Regenerate"), .Pointer = &AMountainGenWorldActor::execRegenerate },
		{ .NameUTF8 = UTF8TEXT("SetSeed"), .Pointer = &AMountainGenWorldActor::execSetSeed },
	};
	static UObject* (*const DependentSingletons[])();
	static constexpr FClassFunctionLinkInfo FuncInfo[] = {
		{ &Z_Construct_UFunction_AMountainGenWorldActor_RandomizeSeed, "RandomizeSeed" }, // 3160146718
		{ &Z_Construct_UFunction_AMountainGenWorldActor_Regenerate, "Regenerate" }, // 79529885
		{ &Z_Construct_UFunction_AMountainGenWorldActor_SetSeed, "SetSeed" }, // 1922364108
	};
	static_assert(UE_ARRAY_COUNT(FuncInfo) < 2048);
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AMountainGenWorldActor>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
}; // struct Z_Construct_UClass_AMountainGenWorldActor_Statics

// ********** Begin Class AMountainGenWorldActor Property Definitions ******************************
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_AMountainGenWorldActor_Statics::NewProp_ProcMesh = { "ProcMesh", nullptr, (EPropertyFlags)0x01140000000a001d, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenWorldActor, ProcMesh), Z_Construct_UClass_UProceduralMeshComponent_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ProcMesh_MetaData), NewProp_ProcMesh_MetaData) };
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_AMountainGenWorldActor_Statics::NewProp_VoxelMaterial = { "VoxelMaterial", nullptr, (EPropertyFlags)0x0114000000000005, UECodeGen_Private::EPropertyGenFlags::Object | UECodeGen_Private::EPropertyGenFlags::ObjectPtr, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenWorldActor, VoxelMaterial), Z_Construct_UClass_UMaterialInterface_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_VoxelMaterial_MetaData), NewProp_VoxelMaterial_MetaData) };
const UECodeGen_Private::FStructPropertyParams Z_Construct_UClass_AMountainGenWorldActor_Statics::NewProp_Settings = { "Settings", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AMountainGenWorldActor, Settings), Z_Construct_UScriptStruct_FMountainGenSettings, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Settings_MetaData), NewProp_Settings_MetaData) }; // 1910493211
void Z_Construct_UClass_AMountainGenWorldActor_Statics::NewProp_bEnableRandomSeedKey_SetBit(void* Obj)
{
	((AMountainGenWorldActor*)Obj)->bEnableRandomSeedKey = 1;
}
const UECodeGen_Private::FBoolPropertyParams Z_Construct_UClass_AMountainGenWorldActor_Statics::NewProp_bEnableRandomSeedKey = { "bEnableRandomSeedKey", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(AMountainGenWorldActor), &Z_Construct_UClass_AMountainGenWorldActor_Statics::NewProp_bEnableRandomSeedKey_SetBit, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_bEnableRandomSeedKey_MetaData), NewProp_bEnableRandomSeedKey_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_AMountainGenWorldActor_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenWorldActor_Statics::NewProp_ProcMesh,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenWorldActor_Statics::NewProp_VoxelMaterial,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenWorldActor_Statics::NewProp_Settings,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AMountainGenWorldActor_Statics::NewProp_bEnableRandomSeedKey,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AMountainGenWorldActor_Statics::PropPointers) < 2048);
// ********** End Class AMountainGenWorldActor Property Definitions ********************************
UObject* (*const Z_Construct_UClass_AMountainGenWorldActor_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_AActor,
	(UObject* (*)())Z_Construct_UPackage__Script_MountainGen,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AMountainGenWorldActor_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_AMountainGenWorldActor_Statics::ClassParams = {
	&AMountainGenWorldActor::StaticClass,
	"Engine",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	FuncInfo,
	Z_Construct_UClass_AMountainGenWorldActor_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	UE_ARRAY_COUNT(FuncInfo),
	UE_ARRAY_COUNT(Z_Construct_UClass_AMountainGenWorldActor_Statics::PropPointers),
	0,
	0x009000A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AMountainGenWorldActor_Statics::Class_MetaDataParams), Z_Construct_UClass_AMountainGenWorldActor_Statics::Class_MetaDataParams)
};
void AMountainGenWorldActor::StaticRegisterNativesAMountainGenWorldActor()
{
	UClass* Class = AMountainGenWorldActor::StaticClass();
	FNativeFunctionRegistrar::RegisterFunctions(Class, MakeConstArrayView(Z_Construct_UClass_AMountainGenWorldActor_Statics::Funcs));
}
UClass* Z_Construct_UClass_AMountainGenWorldActor()
{
	if (!Z_Registration_Info_UClass_AMountainGenWorldActor.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_AMountainGenWorldActor.OuterSingleton, Z_Construct_UClass_AMountainGenWorldActor_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_AMountainGenWorldActor.OuterSingleton;
}
DEFINE_VTABLE_PTR_HELPER_CTOR_NS(, AMountainGenWorldActor);
AMountainGenWorldActor::~AMountainGenWorldActor() {}
// ********** End Class AMountainGenWorldActor *****************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Mountain___copy_Plugins_MountainGen_Source_MountainGen_Public_MountainGenWorldActor_h__Script_MountainGen_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_AMountainGenWorldActor, AMountainGenWorldActor::StaticClass, TEXT("AMountainGenWorldActor"), &Z_Registration_Info_UClass_AMountainGenWorldActor, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AMountainGenWorldActor), 4065302284U) },
	};
}; // Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Mountain___copy_Plugins_MountainGen_Source_MountainGen_Public_MountainGenWorldActor_h__Script_MountainGen_Statics 
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Mountain___copy_Plugins_MountainGen_Source_MountainGen_Public_MountainGenWorldActor_h__Script_MountainGen_4292546013{
	TEXT("/Script/MountainGen"),
	Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Mountain___copy_Plugins_MountainGen_Source_MountainGen_Public_MountainGenWorldActor_h__Script_MountainGen_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Mountain___copy_Plugins_MountainGen_Source_MountainGen_Public_MountainGenWorldActor_h__Script_MountainGen_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0,
};
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
