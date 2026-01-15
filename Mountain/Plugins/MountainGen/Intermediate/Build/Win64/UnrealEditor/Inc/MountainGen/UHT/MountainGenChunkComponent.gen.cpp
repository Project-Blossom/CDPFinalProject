// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "MountainGenChunkComponent.h"
#include "MountainGenChunkTypes.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
static_assert(!UE_WITH_CONSTINIT_UOBJECT, "This generated code can only be compiled with !UE_WITH_CONSTINIT_OBJECT");
void EmptyLinkFunctionForGeneratedCodeMountainGenChunkComponent() {}

// ********** Begin Cross Module References ********************************************************
MOUNTAINGEN_API UClass* Z_Construct_UClass_UMountainGenChunkComponent();
MOUNTAINGEN_API UClass* Z_Construct_UClass_UMountainGenChunkComponent_NoRegister();
MOUNTAINGEN_API UScriptStruct* Z_Construct_UScriptStruct_FChunkCoord();
PROCEDURALMESHCOMPONENT_API UClass* Z_Construct_UClass_UProceduralMeshComponent();
UPackage* Z_Construct_UPackage__Script_MountainGen();
// ********** End Cross Module References **********************************************************

// ********** Begin Class UMountainGenChunkComponent ***********************************************
FClassRegistrationInfo Z_Registration_Info_UClass_UMountainGenChunkComponent;
UClass* UMountainGenChunkComponent::GetPrivateStaticClass()
{
	using TClass = UMountainGenChunkComponent;
	if (!Z_Registration_Info_UClass_UMountainGenChunkComponent.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			TClass::StaticPackage(),
			TEXT("MountainGenChunkComponent"),
			Z_Registration_Info_UClass_UMountainGenChunkComponent.InnerSingleton,
			StaticRegisterNativesUMountainGenChunkComponent,
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
	return Z_Registration_Info_UClass_UMountainGenChunkComponent.InnerSingleton;
}
UClass* Z_Construct_UClass_UMountainGenChunkComponent_NoRegister()
{
	return UMountainGenChunkComponent::GetPrivateStaticClass();
}
struct Z_Construct_UClass_UMountainGenChunkComponent_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "BlueprintSpawnableComponent", "" },
		{ "ClassGroupNames", "MountainGen" },
		{ "HideCategories", "Object LOD Mobility Trigger" },
		{ "IncludePath", "MountainGenChunkComponent.h" },
		{ "ModuleRelativePath", "Public/MountainGenChunkComponent.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Coord_MetaData[] = {
		{ "Category", "MountainGen" },
		{ "ModuleRelativePath", "Public/MountainGenChunkComponent.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_Revision_MetaData[] = {
		{ "Category", "MountainGen" },
		{ "ModuleRelativePath", "Public/MountainGenChunkComponent.h" },
	};
#endif // WITH_METADATA

// ********** Begin Class UMountainGenChunkComponent constinit property declarations ***************
	static const UECodeGen_Private::FStructPropertyParams NewProp_Coord;
	static const UECodeGen_Private::FIntPropertyParams NewProp_Revision;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
// ********** End Class UMountainGenChunkComponent constinit property declarations *****************
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UMountainGenChunkComponent>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
}; // struct Z_Construct_UClass_UMountainGenChunkComponent_Statics

// ********** Begin Class UMountainGenChunkComponent Property Definitions **************************
const UECodeGen_Private::FStructPropertyParams Z_Construct_UClass_UMountainGenChunkComponent_Statics::NewProp_Coord = { "Coord", nullptr, (EPropertyFlags)0x0010000000020015, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UMountainGenChunkComponent, Coord), Z_Construct_UScriptStruct_FChunkCoord, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Coord_MetaData), NewProp_Coord_MetaData) }; // 811009883
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_UMountainGenChunkComponent_Statics::NewProp_Revision = { "Revision", nullptr, (EPropertyFlags)0x0010000000020015, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UMountainGenChunkComponent, Revision), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_Revision_MetaData), NewProp_Revision_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UMountainGenChunkComponent_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UMountainGenChunkComponent_Statics::NewProp_Coord,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UMountainGenChunkComponent_Statics::NewProp_Revision,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UMountainGenChunkComponent_Statics::PropPointers) < 2048);
// ********** End Class UMountainGenChunkComponent Property Definitions ****************************
UObject* (*const Z_Construct_UClass_UMountainGenChunkComponent_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UProceduralMeshComponent,
	(UObject* (*)())Z_Construct_UPackage__Script_MountainGen,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UMountainGenChunkComponent_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UMountainGenChunkComponent_Statics::ClassParams = {
	&UMountainGenChunkComponent::StaticClass,
	"Engine",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_UMountainGenChunkComponent_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_UMountainGenChunkComponent_Statics::PropPointers),
	0,
	0x00B000A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UMountainGenChunkComponent_Statics::Class_MetaDataParams), Z_Construct_UClass_UMountainGenChunkComponent_Statics::Class_MetaDataParams)
};
void UMountainGenChunkComponent::StaticRegisterNativesUMountainGenChunkComponent()
{
}
UClass* Z_Construct_UClass_UMountainGenChunkComponent()
{
	if (!Z_Registration_Info_UClass_UMountainGenChunkComponent.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UMountainGenChunkComponent.OuterSingleton, Z_Construct_UClass_UMountainGenChunkComponent_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UMountainGenChunkComponent.OuterSingleton;
}
UMountainGenChunkComponent::UMountainGenChunkComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
DEFINE_VTABLE_PTR_HELPER_CTOR_NS(, UMountainGenChunkComponent);
UMountainGenChunkComponent::~UMountainGenChunkComponent() {}
// ********** End Class UMountainGenChunkComponent *************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Mountain_Plugins_MountainGen_Source_MountainGen_Public_MountainGenChunkComponent_h__Script_MountainGen_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UMountainGenChunkComponent, UMountainGenChunkComponent::StaticClass, TEXT("UMountainGenChunkComponent"), &Z_Registration_Info_UClass_UMountainGenChunkComponent, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UMountainGenChunkComponent), 2553173239U) },
	};
}; // Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Mountain_Plugins_MountainGen_Source_MountainGen_Public_MountainGenChunkComponent_h__Script_MountainGen_Statics 
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Mountain_Plugins_MountainGen_Source_MountainGen_Public_MountainGenChunkComponent_h__Script_MountainGen_2201577492{
	TEXT("/Script/MountainGen"),
	Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Mountain_Plugins_MountainGen_Source_MountainGen_Public_MountainGenChunkComponent_h__Script_MountainGen_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Mountain_Plugins_MountainGen_Source_MountainGen_Public_MountainGenChunkComponent_h__Script_MountainGen_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0,
};
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
