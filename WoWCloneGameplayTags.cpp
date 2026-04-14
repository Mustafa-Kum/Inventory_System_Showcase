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
	UE_DEFINE_GAMEPLAY_TAG(State_Casting,			"State.Casting");

	// --- INPUT TAGS ---
	UE_DEFINE_GAMEPLAY_TAG(InputTag_ToggleWeapon,	"InputTag.ToggleWeapon");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_ToggleWalk,		"InputTag.ToggleWalk");

	// --- ABILITY TAGS ---
	UE_DEFINE_GAMEPLAY_TAG(Ability_Warcry,		"Ability.Warcry");

	// --- ABILITY FAILURE TAGS ---
	UE_DEFINE_GAMEPLAY_TAG(AbilityFail_Cooldown,		"Ability.Fail.Cooldown");
	UE_DEFINE_GAMEPLAY_TAG(AbilityFail_Cost,			"Ability.Fail.Cost");
	UE_DEFINE_GAMEPLAY_TAG(AbilityFail_NoTarget,		"Ability.Fail.NoTarget");
	UE_DEFINE_GAMEPLAY_TAG(AbilityFail_Interrupted,		"Ability.Fail.Interrupted");
	UE_DEFINE_GAMEPLAY_TAG(AbilityFail_WeaponRequired,	"Ability.Fail.WeaponRequired");
	UE_DEFINE_GAMEPLAY_TAG(AbilityFail_Blocked,			"Ability.Fail.Blocked");

	// --- COOLDOWN TAGS ---
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Warcry,		"Cooldown.Warcry");
}
