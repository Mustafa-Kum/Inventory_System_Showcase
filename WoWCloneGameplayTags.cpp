// WoWClone Centralized Gameplay Tags — Definitions

#include "WoWCloneGameplayTags.h"

namespace WoWCloneTags
{
	// --- CHARACTER STATE TAGS ---
	UE_DEFINE_GAMEPLAY_TAG(State_Unarmed,			"State.Unarmed");
	UE_DEFINE_GAMEPLAY_TAG(State_Armed,				"State.Armed");
	UE_DEFINE_GAMEPLAY_TAG(State_Idle,				"State.Idle");
	UE_DEFINE_GAMEPLAY_TAG(State_Walking,			"State.Walking");
	UE_DEFINE_GAMEPLAY_TAG(State_Running,			"State.Running");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat,			"State.Combat");
	UE_DEFINE_GAMEPLAY_TAG(State_Uncombat,			"State.Uncombat");

	// --- INPUT TAGS ---
	UE_DEFINE_GAMEPLAY_TAG(InputTag_ToggleWeapon,	"InputTag.ToggleWeapon");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_ToggleWalk,		"InputTag.ToggleWalk");
}
