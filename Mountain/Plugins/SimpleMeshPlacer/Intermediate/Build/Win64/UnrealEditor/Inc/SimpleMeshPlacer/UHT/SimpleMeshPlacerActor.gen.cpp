// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SimpleMeshPlacerActor.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

void EmptyLinkFunctionForGeneratedCodeSimpleMeshPlacerActor() {}

// ********** Begin Cross Module References ********************************************************
ENGINE_API UClass* Z_Construct_UClass_AActor();
ENGINE_API UClass* Z_Construct_UClass_UStaticMeshComponent_NoRegister();
SIMPLEMESHPLACER_API UClass* Z_Construct_UClass_ASimpleMeshPlacerActor();
SIMPLEMESHPLACER_API UClass* Z_Construct_UClass_ASimpleMeshPlacerActor_NoRegister();
UPackage* Z_Construct_UPackage__Script_SimpleMeshPlacer();
// ********** End Cross Module References **********************************************************

// ********** Begin Class ASimpleMeshPlacerActor ***************************************************
void ASimpleMeshPlacerActor::StaticRegisterNativesASimpleMeshPlacerActor()
{
}
FClassRegistrationInfo Z_Registration_Info_UClass_ASimpleMeshPlacerActor;
UClass* ASimpleMeshPlacerActor::GetPrivateStaticClass()
{
	using TClass = ASimpleMeshPlacerActor;
	if (!Z_Registration_Info_UClass_ASimpleMeshPlacerActor.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			StaticPackage(),
			TEXT("SimpleMeshPlacerActor"),
			Z_Registration_Info_UClass_ASimpleMeshPlacerActor.InnerSingleton,
			StaticRegisterNativesASimpleMeshPlacerActor,
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
	return Z_Registration_Info_UClass_ASimpleMeshPlacerActor.InnerSingleton;
}
UClass* Z_Construct_UClass_ASimpleMeshPlacerActor_NoRegister()
{
	return ASimpleMeshPlacerActor::GetPrivateStaticClass();
}
struct Z_Construct_UClass_ASimpleMeshPlacerActor_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xc4\xa1\xef\xbf\xbd\xcf\xb8\xef\xbf\xbd StaticMeshComponent \xef\xbf\xbd\xcf\xb3\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xd6\xb4\xef\xbf\xbd \xef\xbf\xbd\xdc\xbc\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\n * - \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xd9\xb2\xd9\xb0\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd Details\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd MeshComponent\xef\xbf\xbd\xef\xbf\xbd Static Mesh\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xd9\xb2\xd9\xb8\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\n */" },
#endif
		{ "IncludePath", "SimpleMeshPlacerActor.h" },
		{ "ModuleRelativePath", "Public/SimpleMeshPlacerActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xc4\xa1\xef\xbf\xbd\xcf\xb8\xef\xbf\xbd StaticMeshComponent \xef\xbf\xbd\xcf\xb3\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xd6\xb4\xef\xbf\xbd \xef\xbf\xbd\xdc\xbc\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\n- \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xd9\xb2\xd9\xb0\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd Details\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd MeshComponent\xef\xbf\xbd\xef\xbf\xbd Static Mesh\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xd9\xb2\xd9\xb8\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd" },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_MeshComponent_MetaData[] = {
		{ "Category", "Mesh" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// \xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xc6\xae\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xde\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xd2\xb4\xef\xbf\xbd\xef\xbf\xbd\xd8\xbc\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\n" },
#endif
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "Public/SimpleMeshPlacerActor.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xc6\xae\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xde\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd \xef\xbf\xbd\xd2\xb4\xef\xbf\xbd\xef\xbf\xbd\xd8\xbc\xef\xbf\xbd \xef\xbf\xbd\xef\xbf\xbd\xef\xbf\xbd" },
#endif
	};
#endif // WITH_METADATA
	static const UECodeGen_Private::FObjectPropertyParams NewProp_MeshComponent;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<ASimpleMeshPlacerActor>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
};
const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_ASimpleMeshPlacerActor_Statics::NewProp_MeshComponent = { "MeshComponent", nullptr, (EPropertyFlags)0x00200800000a001d, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(ASimpleMeshPlacerActor, MeshComponent), Z_Construct_UClass_UStaticMeshComponent_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_MeshComponent_MetaData), NewProp_MeshComponent_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_ASimpleMeshPlacerActor_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ASimpleMeshPlacerActor_Statics::NewProp_MeshComponent,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_ASimpleMeshPlacerActor_Statics::PropPointers) < 2048);
UObject* (*const Z_Construct_UClass_ASimpleMeshPlacerActor_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_AActor,
	(UObject* (*)())Z_Construct_UPackage__Script_SimpleMeshPlacer,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_ASimpleMeshPlacerActor_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_ASimpleMeshPlacerActor_Statics::ClassParams = {
	&ASimpleMeshPlacerActor::StaticClass,
	"Engine",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_ASimpleMeshPlacerActor_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_ASimpleMeshPlacerActor_Statics::PropPointers),
	0,
	0x009001A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_ASimpleMeshPlacerActor_Statics::Class_MetaDataParams), Z_Construct_UClass_ASimpleMeshPlacerActor_Statics::Class_MetaDataParams)
};
UClass* Z_Construct_UClass_ASimpleMeshPlacerActor()
{
	if (!Z_Registration_Info_UClass_ASimpleMeshPlacerActor.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_ASimpleMeshPlacerActor.OuterSingleton, Z_Construct_UClass_ASimpleMeshPlacerActor_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_ASimpleMeshPlacerActor.OuterSingleton;
}
DEFINE_VTABLE_PTR_HELPER_CTOR(ASimpleMeshPlacerActor);
ASimpleMeshPlacerActor::~ASimpleMeshPlacerActor() {}
// ********** End Class ASimpleMeshPlacerActor *****************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Mountain_Plugins_SimpleMeshPlacer_Source_SimpleMeshPlacer_Public_SimpleMeshPlacerActor_h__Script_SimpleMeshPlacer_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_ASimpleMeshPlacerActor, ASimpleMeshPlacerActor::StaticClass, TEXT("ASimpleMeshPlacerActor"), &Z_Registration_Info_UClass_ASimpleMeshPlacerActor, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(ASimpleMeshPlacerActor), 845468823U) },
	};
};
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Mountain_Plugins_SimpleMeshPlacer_Source_SimpleMeshPlacer_Public_SimpleMeshPlacerActor_h__Script_SimpleMeshPlacer_2099895366(TEXT("/Script/SimpleMeshPlacer"),
	Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Mountain_Plugins_SimpleMeshPlacer_Source_SimpleMeshPlacer_Public_SimpleMeshPlacerActor_h__Script_SimpleMeshPlacer_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_User_Documents_Unreal_Projects_Mountain_Plugins_SimpleMeshPlacer_Source_SimpleMeshPlacer_Public_SimpleMeshPlacerActor_h__Script_SimpleMeshPlacer_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0);
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
