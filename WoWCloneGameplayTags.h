// WoWClone Centralized Gameplay Tags (AAA Standard: Single Source of Truth)

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 * Central registry of all native Gameplay Tags used in C++ code.
 * Tags are declared here (UE_DECLARE_GAMEPLAY_TAG_EXTERN) and defined in the .cpp.
 * This eliminates string-literal duplication and prevents runtime typo bugs.
 */
namespace WoWCloneTags
{
	// --- CHARACTER STATE TAGS ---
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Unarmed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Armed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Idle);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Walking);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Running);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Uncombat);

	// --- INPUT TAGS ---
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_ToggleWeapon);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_ToggleWalk);
}
