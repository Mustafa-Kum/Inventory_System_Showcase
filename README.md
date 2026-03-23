# Building My RPG — Part 1: Inventory System Showcase (Unreal Engine 5)

![Unreal Engine](https://img.shields.io/badge/Unreal_Engine-5.0%2B-blue?logo=unrealengine&logoColor=white)
![C++](https://img.shields.io/badge/C%2B%2B-High_Performance-blue)
![Architecture](https://img.shields.io/badge/Architecture-Modular-green)
![System](https://img.shields.io/badge/System-Data--Driven-orange)

## 📌 Important Note

**This is not a plug-and-play system.** This code was extracted from a larger project and simplified to showcase architectural decisions. It is not intended to be integrated directly into a fresh project without proper context and adaptation.

---

<div align="center">
  <img src="Gifs/ItemDetail.gif" width="80%">
  <p><em>Dynamic tooltip driven natively by Gameplay Effect CDOs.</em></p>
  
  <br>

  <img src="Gifs/ItemEquip.gif" width="80%">
  <p><em>Equipping items with instant UI updates and skeletal mesh synchronization.</em></p>

  <br>

  <img src="Gifs/ItemUnequip.gif" width="80%">
  <p><em>Unequipping flow with secure state and GAS handle invalidation.</em></p>

  <br>
  <p><em>Event-driven UI updates, drag & drop interactions, and dynamic parsing systems.</em></p>
</div>

---

## 📖 Overview

This project is a **C++ code showcase** demonstrating a modular and scalable **Inventory & Equipment System** built for Unreal Engine 5.

The primary focus is on **architecture and engineering quality**, rather than feature completeness.

The system is designed to:
* Scale efficiently with increasing complexity.
* Remain easy to maintain with strict adherence to the **DRY (Don't Repeat Yourself)** principle.
* Prevent unnecessary runtime costs and state-related bugs.

---

## 🧩 Architecture

The system is built on four core principles:

* **Event-Driven Communication** → No polling, no wasted CPU cycles.
* **Data-Driven Design** → No hardcoded gameplay data.
* **Separation of Responsibilities** → Clean, well-defined system boundaries.
* **Asynchronous Workflows** → No game-thread blocking during asset loads.

---

## ✨ Key Features

### 🔄 Event-Driven UI
* Zero `Tick` usage in the UI.
* Delegate-based updates (e.g., `OnInventoryUpdated`).
* UI rebuilds occur *only* when necessary.

### 📊 Data-Driven System & CDO Parsing
* Scalable hierarchy mapping from a base `UItemDataAsset`.
* **DRY Compliance:** Tooltips natively parse `UGameplayEffect` Class Default Objects (CDOs) to extract modifiers at runtime, eliminating data duplication.
* Fully designer-friendly structure.

### 🛡️ Modular Equipment & AAA Rulesets
* Dynamic `USkeletalMeshComponent` assignments utilizing `SetLeaderPoseComponent` for seamless modular armor integration.
* Intelligent conflict resolution (e.g., equipping a Two-Handed weapon automatically unequips the off-hand shield).
* Drag-and-drop operations strictly validated against `EEquipmentSlot` mappings.

### ⚡ Asynchronous Asset Loading
* Utilizes Unreal’s `FStreamableManager`.
* Loads meshes and icons on-demand.
* Prevents frame hitches during execution.

### 🧬 Gameplay Ability System (GAS) Integration
* **Secure Buff Management:** Safely tracks native `FActiveGameplayEffectHandle` arrays to apply, stack, and strip complex percentages and item buffs dynamically without floating-point math duplication.
* Attribute Sets handle core stat calculations.
* Gameplay Tags define and drive states (e.g., `State.Armed`).

### 🖱️ Drag & Drop System
* Native UMG drag & drop functionality.
* Payload-based operations.
* Clear source/target validation handling.

---

## 📂 Project Structure

All core files are placed in the root directory for fast review.

### Core Systems
* `InventoryComponent.cpp` → State & logic management, 2H rulesets.
* `CharacterBase.cpp` → Modular equipment handling & GAS stat delegation.

### UI Layer
* `InventoryWidget.cpp` → UI orchestration & map bindings.
* `WeaponSlotWidget.cpp` → Slot behavior & async icon loading.
* `ItemTooltipWidget.cpp` → Advanced CDO parsing for dynamic item details.
* `StatRowWidget.cpp` → Reusable UI elements for displaying stats.

### Data Layer
* `UItemDataAsset` → Polymorphic base wrapper for all ecosystem items.
* `UWeaponDataAsset` → Dedicated weapon definitions.
* `UCharacterDataAsset` → Character configuration.

### Supporting Systems
* `UAttributeSet` → GAS stat calculations.
* `FGameplayTag` → Global state definitions.
* `UDragDropOperation` → Abstract interaction payloads.

---

## 🧠 How It Works

1.  **Interaction:** The player interacts with the inventory (drag/drop, equip, inspect).
2.  **Validation:** The `InventoryComponent` updates its internal state and validates 2H / Slot rules.
3.  **GAS Injection:** The system fetches the `UGameplayEffect` within the item and injects an active stat handle through the `AbilitySystemComponent`.
4.  **Broadcast:** The `OnInventoryUpdated` delegate is broadcast to the listening HUD.
5.  **UI Update:** The UI listens for this event and rebuilds *only* the affected elements (e.g., the Tooltip dynamically parses CDO Modifiers).
6.  **Mesh Sync:** Modular meshes are synchronized into the main character body.

---

## 🏗️ Design Philosophy

> *Systems should remain stable as complexity grows.*

**This system avoids:**
* Hidden dependencies.
* Unnecessary framework updates.
* Hardcoded logic & data duplication.

**And prioritizes:**
* Scalability.
* Maintainability.
* Performance.

---

## 👨‍💻 Developer

**Mustafa Kum** Gameplay Programmer (C++ / Unreal Engine)
