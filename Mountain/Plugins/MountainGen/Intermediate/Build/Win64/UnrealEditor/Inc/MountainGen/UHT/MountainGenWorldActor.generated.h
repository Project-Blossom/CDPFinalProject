// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "MountainGenWorldActor.h"

#ifdef MOUNTAINGEN_MountainGenWorldActor_generated_h
#error "MountainGenWorldActor.generated.h already included, missing '#pragma once' in MountainGenWorldActor.h"
#endif
#define MOUNTAINGEN_MountainGenWorldActor_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin Class AMountainGenWorldActor ***************************************************
#define FID_Users_User_Documents_Unreal_Projects_Mountain___copy_Plugins_MountainGen_Source_MountainGen_Public_MountainGenWorldActor_h_14_RPC_WRAPPERS_NO_PURE_DECLS \
	DECLARE_FUNCTION(execRandomizeSeed); \
	DECLARE_FUNCTION(execSetSeed); \
	DECLARE_FUNCTION(execRegenerate);


struct Z_Construct_UClass_AMountainGenWorldActor_Statics;
MOUNTAINGEN_API UClass* Z_Construct_UClass_AMountainGenWorldActor_NoRegister();

#define FID_Users_User_Documents_Unreal_Projects_Mountain___copy_Plugins_MountainGen_Source_MountainGen_Public_MountainGenWorldActor_h_14_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesAMountainGenWorldActor(); \
	friend struct ::Z_Construct_UClass_AMountainGenWorldActor_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend MOUNTAINGEN_API UClass* ::Z_Construct_UClass_AMountainGenWorldActor_NoRegister(); \
public: \
	DECLARE_CLASS2(AMountainGenWorldActor, AActor, COMPILED_IN_FLAGS(0 | CLASS_Config), CASTCLASS_None, TEXT("/Script/MountainGen"), Z_Construct_UClass_AMountainGenWorldActor_NoRegister) \
	DECLARE_SERIALIZER(AMountainGenWorldActor)


#define FID_Users_User_Documents_Unreal_Projects_Mountain___copy_Plugins_MountainGen_Source_MountainGen_Public_MountainGenWorldActor_h_14_ENHANCED_CONSTRUCTORS \
	/** Deleted move- and copy-constructors, should never be used */ \
	AMountainGenWorldActor(AMountainGenWorldActor&&) = delete; \
	AMountainGenWorldActor(const AMountainGenWorldActor&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, AMountainGenWorldActor); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(AMountainGenWorldActor); \
	DEFINE_DEFAULT_CONSTRUCTOR_CALL(AMountainGenWorldActor) \
	NO_API virtual ~AMountainGenWorldActor();


#define FID_Users_User_Documents_Unreal_Projects_Mountain___copy_Plugins_MountainGen_Source_MountainGen_Public_MountainGenWorldActor_h_11_PROLOG
#define FID_Users_User_Documents_Unreal_Projects_Mountain___copy_Plugins_MountainGen_Source_MountainGen_Public_MountainGenWorldActor_h_14_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_Users_User_Documents_Unreal_Projects_Mountain___copy_Plugins_MountainGen_Source_MountainGen_Public_MountainGenWorldActor_h_14_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_Users_User_Documents_Unreal_Projects_Mountain___copy_Plugins_MountainGen_Source_MountainGen_Public_MountainGenWorldActor_h_14_INCLASS_NO_PURE_DECLS \
	FID_Users_User_Documents_Unreal_Projects_Mountain___copy_Plugins_MountainGen_Source_MountainGen_Public_MountainGenWorldActor_h_14_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class AMountainGenWorldActor;

// ********** End Class AMountainGenWorldActor *****************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Users_User_Documents_Unreal_Projects_Mountain___copy_Plugins_MountainGen_Source_MountainGen_Public_MountainGenWorldActor_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
