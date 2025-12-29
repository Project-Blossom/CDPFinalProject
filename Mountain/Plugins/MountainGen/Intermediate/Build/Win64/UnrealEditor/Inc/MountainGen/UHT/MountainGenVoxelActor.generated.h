// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "MountainGenVoxelActor.h"

#ifdef MOUNTAINGEN_MountainGenVoxelActor_generated_h
#error "MountainGenVoxelActor.generated.h already included, missing '#pragma once' in MountainGenVoxelActor.h"
#endif
#define MOUNTAINGEN_MountainGenVoxelActor_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin Class AMountainGenVoxelActor ***************************************************
#define FID_Users_User_Documents_Unreal_Projects_b1234_Plugins_MountainGen_Source_MountainGen_Public_MountainGenVoxelActor_h_13_RPC_WRAPPERS_NO_PURE_DECLS \
	DECLARE_FUNCTION(execSetSeed); \
	DECLARE_FUNCTION(execRegenerate);


MOUNTAINGEN_API UClass* Z_Construct_UClass_AMountainGenVoxelActor_NoRegister();

#define FID_Users_User_Documents_Unreal_Projects_b1234_Plugins_MountainGen_Source_MountainGen_Public_MountainGenVoxelActor_h_13_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesAMountainGenVoxelActor(); \
	friend struct Z_Construct_UClass_AMountainGenVoxelActor_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend MOUNTAINGEN_API UClass* Z_Construct_UClass_AMountainGenVoxelActor_NoRegister(); \
public: \
	DECLARE_CLASS2(AMountainGenVoxelActor, AActor, COMPILED_IN_FLAGS(0 | CLASS_Config), CASTCLASS_None, TEXT("/Script/MountainGen"), Z_Construct_UClass_AMountainGenVoxelActor_NoRegister) \
	DECLARE_SERIALIZER(AMountainGenVoxelActor)


#define FID_Users_User_Documents_Unreal_Projects_b1234_Plugins_MountainGen_Source_MountainGen_Public_MountainGenVoxelActor_h_13_ENHANCED_CONSTRUCTORS \
	/** Deleted move- and copy-constructors, should never be used */ \
	AMountainGenVoxelActor(AMountainGenVoxelActor&&) = delete; \
	AMountainGenVoxelActor(const AMountainGenVoxelActor&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, AMountainGenVoxelActor); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(AMountainGenVoxelActor); \
	DEFINE_DEFAULT_CONSTRUCTOR_CALL(AMountainGenVoxelActor) \
	NO_API virtual ~AMountainGenVoxelActor();


#define FID_Users_User_Documents_Unreal_Projects_b1234_Plugins_MountainGen_Source_MountainGen_Public_MountainGenVoxelActor_h_10_PROLOG
#define FID_Users_User_Documents_Unreal_Projects_b1234_Plugins_MountainGen_Source_MountainGen_Public_MountainGenVoxelActor_h_13_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_Users_User_Documents_Unreal_Projects_b1234_Plugins_MountainGen_Source_MountainGen_Public_MountainGenVoxelActor_h_13_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_Users_User_Documents_Unreal_Projects_b1234_Plugins_MountainGen_Source_MountainGen_Public_MountainGenVoxelActor_h_13_INCLASS_NO_PURE_DECLS \
	FID_Users_User_Documents_Unreal_Projects_b1234_Plugins_MountainGen_Source_MountainGen_Public_MountainGenVoxelActor_h_13_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class AMountainGenVoxelActor;

// ********** End Class AMountainGenVoxelActor *****************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Users_User_Documents_Unreal_Projects_b1234_Plugins_MountainGen_Source_MountainGen_Public_MountainGenVoxelActor_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
