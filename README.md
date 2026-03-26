# Building My RPG Part 2 — Inventory & Equipment System Showcase (Unreal Engine 5 / GAS)

![Unreal Engine](https://img.shields.io/badge/Unreal_Engine-5.0%2B-blue?logo=unrealengine&logoColor=white)
![C++](https://img.shields.io/badge/C%2B%2B-Gameplay_Programming-blue)
![Architecture](https://img.shields.io/badge/Architecture-Modular-green)
![System](https://img.shields.io/badge/System-Event--Driven-orange)
![GAS](https://img.shields.io/badge/GAS-Integrated-purple)

## 📌 Important Note

**This is not a plug-and-play package.**  
This code was extracted from a larger Unreal Engine 5 RPG project and shared as a **reference implementation** for architecture, system boundaries, and gameplay engineering decisions.

It is designed to be studied, adapted, and transplanted into another project with context, not dropped into a blank project without modification.

---

<div align="center">
  <img src="Gifs/ItemDetail.gif" width="80%">
  <p><em>Tooltip generation driven from item data and Gameplay Effect modifier definitions.</em></p>

  <br>

  <img src="Gifs/ItemEquip.gif" width="80%">
  <p><em>Equipping items with immediate UI refresh, stat application, and modular mesh updates.</em></p>

  <br>

  <img src="Gifs/ItemUnequip.gif" width="80%">
  <p><em>Unequipping flow with slot validation, handle-based effect removal, and visual sync.</em></p>
</div>

---

## 📖 Overview

This repository showcases a modular **Inventory & Equipment System** built in **C++ for Unreal Engine 5**, integrated with the **Gameplay Ability System (GAS)**.

The focus is not just on “making an inventory work,” but on structuring the system so it stays understandable and maintainable as the project grows.

This module is built around a few core goals:

- Keep inventory state management separate from UI
- Use event-driven refreshes instead of ticking widgets
- Drive item behavior from assets rather than duplicating values in multiple places
- Apply and remove item stats safely through GAS
- Support modular character visuals and weapon state transitions without blocking the game thread

---

## 🧩 What This System Covers

This showcase includes:

- Backpack slot management
- Equipment slot management
- Right-click equip / unequip / consume flows
- Drag & drop between inventory and equipment
- Two-handed weapon constraint handling
- Tooltip generation from item data and Gameplay Effect modifiers
- Async loading for icons, equipment meshes, weapon meshes, and montages
- GAS-based stat application / removal
- Modular armor mesh synchronization on the character

---

## 🏗️ Architecture

At a high level, the flow looks like this:

```text
[ UItemDataAsset / UWeaponDataAsset ]
                ↓
[ UInventoryComponent ]  ← Owns inventory state, equip rules, stacking, validation
                ↓
[ ACharacterBase ]       ← Applies/removes GAS effects and updates visual equipment state
                ↓
[ UInventoryWidget ]     ← Listens to inventory update events and refreshes UI
                ↓
[ UItemSlotWidget ]      ← Handles slot visuals, drag/drop, right-click actions
                ↓
[ UItemTooltipWidget ]   ← Builds tooltip rows from item data and GE modifier data
```

Each layer has a focused responsibility:

- `UInventoryComponent` owns the rules and authoritative item state
- `ACharacterBase` owns mesh changes, weapon visuals, and GAS application/removal
- `UInventoryWidget` rebuilds the inventory presentation when state changes
- `UItemSlotWidget` handles direct player interactions
- `UItemTooltipWidget` converts item definitions into readable UI

---

## ✨ Key Features

### 🔄 Event-Driven UI Refresh

The inventory UI does **not** rely on `Tick`.

Instead, `UInventoryWidget` binds to `OnInventoryUpdated` and refreshes when inventory state changes:

- equipping
- unequipping
- consuming
- swapping
- stacking
- any slot mutation that broadcasts an inventory update

This keeps the UI simple and predictable.

Important detail: the current implementation refreshes the inventory UI **on inventory events**, not per-frame and not per-attribute delegate. It repopulates the equipment/backpack presentation and updates the stat panel when the inventory component broadcasts.

---

### 🖱️ Drag & Drop + Right-Click Interaction

`UItemSlotWidget` supports:

- Left-click drag detection
- Drag from inventory to equipment
- Drag from equipment to inventory
- Drag between inventory slots
- Right-click equip
- Right-click unequip
- Right-click consume

All drag/drop actions are validated against slot context and `EEquipmentSlot` compatibility before backend logic is executed.

This keeps validation close to the interaction layer while still routing the actual state changes through `UInventoryComponent`.

---

### 🧠 Data-Driven Item Definitions

Items are defined through `UPrimaryDataAsset`-based assets:

- `UItemDataAsset`
- `UWeaponDataAsset`

Data lives in asset structures such as:

- item name
- rarity
- icon
- equipment mesh
- valid equipment slot
- sell price
- max stack size
- equipped stat effect
- weapon-specific data such as damage, weapon speed, montages, sockets, and mesh

This keeps most gameplay-facing configuration in data assets rather than duplicated across UI and code.

Note: the system is **primarily** data-driven, but `UWeaponDataAsset` still assigns a few sensible weapon defaults in C++ (`ItemType`, `ValidEquipmentSlot`, `MaxStackSize`).

---

### 📊 Tooltip Parsing from Gameplay Effect Definitions

One of the most useful parts of the system is the tooltip flow.

Instead of duplicating bonus stat text manually on the item asset, `UItemTooltipWidget` reads modifier data from the item’s `EquippedStatEffect` and builds tooltip rows dynamically.

That means:

- the item backend remains the source of truth
- the UI does not need a second bonus-stat schema
- changing a Gameplay Effect updates gameplay and tooltip output together

The tooltip currently reads modifier definitions from the Gameplay Effect CDO and displays magnitudes that can be resolved from those definitions at runtime.

---

### 🛡️ Equipment Rules and Two-Handed Constraints

`UInventoryComponent` contains the core equip validation logic.

A notable example is the two-handed weapon rule set:

- Equipping a two-handed main-hand weapon can force the off-hand item out
- Trying to equip an off-hand item while a two-handed weapon is equipped can unequip the main-hand weapon first

This keeps weapon rules centralized in the inventory backend instead of scattering them across UI widgets.

---

### ⚡ Async Asset Loading

The system uses Unreal’s async loading path through `UAssetManager::GetStreamableManager().RequestAsyncLoad(...)`.

It asynchronously loads:

- item icons
- weapon meshes
- equipment meshes
- equip / unequip montages

This helps avoid blocking loads during gameplay-facing interactions.

---

### 🧬 GAS Integration

When an equippable item is mounted, the system:

1. creates a Gameplay Effect spec from the item’s `EquippedStatEffect`
2. applies it to the owning character’s `AbilitySystemComponent`
3. stores the returned `FActiveGameplayEffectHandle`

When the item is removed, the stored handle is used to remove the active effect cleanly.

This gives the system a reliable backend path for stat application and removal without duplicating stat math in the inventory layer.

Important detail: the current implementation tracks **one active effect handle per equipped slot**, stored in a `TMap<EEquipmentSlot, FActiveGameplayEffectHandle>`.

---

### 🧩 Modular Character Equipment Visuals

Visual equipment sync is handled in `ACharacterBase`.

For supported armor pieces, dedicated `USkeletalMeshComponent`s follow the main mesh using `SetLeaderPoseComponent`, allowing armor visuals to stay synchronized with character animation.

Current modular mesh support in this showcase includes:

- Helm
- Chest
- Gauntlets
- Leggings
- Boots

Weapon visuals are handled separately, including:

- pending weapon mesh setup
- holster / equip socket switching
- montage-driven draw / holster transitions

---

## 📂 Relevant Files

### Core Logic

- `InventoryComponent.cpp`
  - Inventory state, stacking, equip/unequip rules, 2H constraints, broadcasts
- `CharacterBase.cpp`
  - Item stat application/removal, active GE handle tracking, modular mesh updates, weapon mesh loading

### UI Layer

- `InventoryWidget.cpp`
  - Inventory event binding, slot refresh, stat panel refresh
- `ItemSlotWidget.cpp`
  - Slot visuals, drag/drop handling, right-click actions, async icon loading
- `ItemTooltipWidget.cpp`
  - Tooltip setup, rarity styling, base stat display, Gameplay Effect modifier parsing
- `StatRowWidget.cpp`
  - Reusable tooltip stat row widget

### Data Layer

- `ItemDataAsset.h`
  - Base item definitions, shared item metadata, inventory item struct
- `WeaponDataAsset.h`
  - Weapon-specific data such as damage, speed, mesh, montages, sockets

### Supporting Types

- `ItemDragDropOperation.h`
  - Drag/drop payload data between widgets
- `CharacterAttributeSet.h`
  - Attribute definitions used by the stat UI
- `WoWCloneGameplayTags.h`
  - Gameplay tags such as armed / unarmed state

---

## 🧠 How It Works

1. **Player Interaction**  
   The player clicks, drags, drops, equips, unequips, or consumes an item through `UItemSlotWidget`.

2. **Validation**  
   The slot widget validates context and payload shape, then forwards the action to `UInventoryComponent`.

3. **Inventory State Update**  
   `UInventoryComponent` processes the request:
   - slot validation
   - stack logic
   - equip / unequip flow
   - two-handed weapon constraints
   - inventory swapping

4. **Gameplay State Application**  
   If the action changes equipment, `ACharacterBase` applies or removes the item’s Gameplay Effect and updates visual equipment state.

5. **Broadcast**  
   `UInventoryComponent` broadcasts `OnInventoryUpdated`.

6. **UI Refresh**  
   `UInventoryWidget` responds by refreshing equipment slots, backpack slots, and the stat panel.

7. **Tooltip Rendering**  
   Hovering an item spawns or updates a tooltip widget, which reads item data and Gameplay Effect modifier info to build readable stat rows.

---

## ✅ Requirements / Assumptions

This showcase assumes a project setup with:

- Unreal Engine 5
- C++
- UMG
- Gameplay Ability System
- Gameplay Tags
- A character architecture compatible with `ACharacterBase`
- Item definitions authored as `UPrimaryDataAsset` assets

The inventory UI also assumes Blueprint widgets with the expected `BindWidget` names for:

- equipment slots
- inventory grid
- stat text fields

---

## 📌 Current Scope

This repository focuses on the inventory/equipment slice only.

It currently demonstrates:

- inventory slots
- equipment slots
- item effects
- visual sync
- weapon transitions
- tooltip generation
- event-driven UI refresh

It does **not** try to solve every inventory-adjacent feature yet.

For example, this code does not fully implement:

- persistence / save system
- multiplayer replication rules
- vendor economy loop
- character preview rendering logic
- weight / currency logic beyond placeholder UI bindings

Those belong to the larger game architecture around this module.

---

## 🏗️ Design Philosophy

> Systems should stay readable as they grow.

This implementation prioritizes:

- clear ownership boundaries
- event-driven UI over widget ticking
- data-driven item authoring
- reusable interaction flows
- safe GAS integration
- async loading for gameplay-facing assets
- minimal duplication between gameplay data and UI representation

Or put simply:

- the inventory owns inventory rules
- the character owns character state and visuals
- the UI listens and renders
- the item assets define the content

---

## 📄 License

Provided as a reference implementation for Unreal Engine 5 inventory / equipment architecture built around GAS-style gameplay systems.

Feel free to study it, adapt it, and use it as a foundation for your own project.
