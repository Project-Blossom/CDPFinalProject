// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "MountainGenSettings.h"

#ifdef MOUNTAINGEN_MountainGenSettings_generated_h
#error "MountainGenSettings.generated.h already included, missing '#pragma once' in MountainGenSettings.h"
#endif
#define MOUNTAINGEN_MountainGenSettings_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin ScriptStruct FMountainGenSettings **********************************************
struct Z_Construct_UScriptStruct_FMountainGenSettings_Statics;
#define FID_Users_User_Documents_Unreal_Projects_Prototype_Plugins_MountainGen_Source_MountainGen_Public_MountainGenSettings_h_17_GENERATED_BODY \
	friend struct ::Z_Construct_UScriptStruct_FMountainGenSettings_Statics; \
	MOUNTAINGEN_API static class UScriptStruct* StaticStruct();


struct FMountainGenSettings;
// ********** End ScriptStruct FMountainGenSettings ************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Users_User_Documents_Unreal_Projects_Prototype_Plugins_MountainGen_Source_MountainGen_Public_MountainGenSettings_h

// ********** Begin Enum EMountainGenDifficulty ****************************************************
#define FOREACH_ENUM_EMOUNTAINGENDIFFICULTY(op) \
	op(EMountainGenDifficulty::Easy) \
	op(EMountainGenDifficulty::Normal) \
	op(EMountainGenDifficulty::Hard) \
	op(EMountainGenDifficulty::Extreme) 

enum class EMountainGenDifficulty : uint8;
template<> struct TIsUEnumClass<EMountainGenDifficulty> { enum { Value = true }; };
template<> MOUNTAINGEN_NON_ATTRIBUTED_API UEnum* StaticEnum<EMountainGenDifficulty>();
// ********** End Enum EMountainGenDifficulty ******************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
