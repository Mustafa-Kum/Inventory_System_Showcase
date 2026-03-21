# 🧠 Inventory System Showcase (Unreal Engine 5)
![Unreal Engine](https://img.shields.io/badge/Unreal_Engine-5.0%2B-blue?logo=unrealengine&logoColor=white)
![C++](https://img.shields.io/badge/C%2B%2B-High_Performance-blue)
![Architecture](https://img.shields.io/badge/Architecture-Modular-green)
![System](https://img.shields.io/badge/System-Data--Driven-orange)
## 📌 Important Note
This is not a plug-and-play system.
The code is extracted from a larger project and simplified to showcase architectural decisions, not to be directly integrated into a fresh project without context.
---
<div align="center">
  <div align="center">
  <img src="Gifs/ItemDetail.gif" width="80%">
  <p><em>Dynamic tooltip driven natively by Gameplay Effect CDOs.</em></p>
  
  <img src="Gifs/ItemEquip.gif" width="80%">
  <p><em>Equipping items with instant UI update and skeletal mesh synchronization.</em></p>
  <img src="Gifs/ItemUnequip.gif" width="80%">
  <p><em>Unequipping flow with secure state and GAS handle invalidation.</em></p>
</div>
  <p><em>Event-driven UI updates, drag & drop interactions, and dynamic parsing systems.</em></p>
</div>
---
## 📖 Overview
This project is a **C++ code showcase** demonstrating a modular and scalable **Inventory & Equipment System** built for Unreal Engine 5.
The focus is on **architecture and engineering quality**, not feature completeness.
The system is designed to:
- Scale with increasing complexity  
- Remain easy to maintain & strict to the DRY principle
- Avoid unnecessary runtime cost & state bugs
---
## 🧩 Architecture
The system is built on a few core principles:
- **Event-Driven Communication** → No polling, no wasted CPU  
- **Data-Driven Design** → No hardcoded gameplay data  
- **Separation of Responsibilities** → Clean system boundaries  
- **Asynchronous Workflows** → No game thread blocking  
---
## ✨ Key Features
### 🔄 Event-Driven UI
- No [Tick](cci:1://file:///d:/WoWClone/Source/WoWClone/Private/Characters/HeroCharacter.cpp:88:0-96:1) usage in UI
- Delegate-based updates (`OnInventoryUpdated`)
- UI rebuilds only when necessary
---
### 📊 Data-Driven System & CDO Parsing
- Scalable hierarchy mapping from a base `UItemDataAsset`.
- **DRY Compliance:** Tooltips natively parse `UGameplayEffect` Class Default Objects (CDOs) to extract modifiers at runtime, eliminating the need to type item stats twice.
- Fully designer-friendly structure.
---
### 🛡️ Modular Equipment & AAA Rulesets
- Dynamic `USkeletalMeshComponent` assignments utilizing `SetLeaderPoseComponent` for seamless modular armor.
- Intelligent conflict resolution (e.g., Equipping a Two-Handed weapon automatically unequips off-hand shields).
- Drag-and-drop operations strictly validated against `EEquipmentSlot` mapping.
---
### ⚡ Asynchronous Asset Loading
- Uses Unreal’s `StreamableManager`
- Loads meshes and icons on demand
- Prevents frame hitches during execution
---
### 🧬 Gameplay Ability System (GAS) Integration
- **Secure Buff Management:** Tracks native `FActiveGameplayEffectHandle` arrays safely to apply, stack, and strip complex percentages and item buffs dynamically without floating-point math duplication.
- Attribute Sets handle core stat calculations.
- Gameplay Tags define states (e.g. `State.Armed`).
---
### 🖱️ Drag & Drop System
- Native Unreal drag & drop
- Payload-based operations
- Clear source/target handling
---
## 📂 Project Structure
All core files are placed in the root directory for fast review.
### Core Systems
- [InventoryComponent](cci:1://file:///d:/WoWClone/Source/WoWClone/Private/Components/InventoryComponent.cpp:10:0-13:1) → State & logic management, 2H rulesets.
- [CharacterBase](cci:1://file:///d:/WoWClone/Source/WoWClone/Private/Characters/CharacterBase.cpp:10:0-45:1) → Modular equipment handling & GAS stat delegation.
### UI Layer
- [InventoryWidget](cci:1://file:///d:/WoWClone/Source/WoWClone/Private/Characters/HeroCharacter.cpp:326:0-340:1) → UI orchestration & map bindings.
- `WeaponSlotWidget` → Slot behavior & async icon loading.
- [ItemTooltipWidget](cci:1://file:///d:/WoWClone/Source/WoWClone/Private/UI/ItemTooltipWidget.cpp:10:0-22:1) → Advanced CDO parsing for dynamic item details.
- `StatRowWidget` → Reusable UI element.
### Data Layer
- `ItemDataAsset` → Polymorphic base wrapper for all ecosystem items.
- `WeaponDataAsset` → Dedicated weapon definitions.
- `CharacterDataAsset` → Character configuration.
### Supporting Systems
- `AttributeSet` → GAS stat calculations.
- `GameplayTags` → Global state definitions.
- `DragDropOperation` → Interaction abstract payload.
---
## 🧠 How It Works
1. Player interacts with inventory (drag/drop, equip, inspect).
2. Inventory Component updates internal state and validates 2H / Slot rules.
3. System fetches the `UGameplayEffect` within the item and injects an active stat handle through the [AbilitySystemComponent](cci:1://file:///d:/WoWClone/Source/WoWClone/Private/Characters/CharacterBase.cpp:72:0-75:1).
4. Delegate (`OnInventoryUpdated`) is broadcasted to the listening HUD.
5. UI listens and rebuilt only affected elements (Tooltip dynamically parses CDO Modifiers).
6. Modular meshes are synchronised into the main character body.
---
## 🧠 Design Philosophy
> Systems should remain stable as complexity grows.
This system avoids:
- Hidden dependencies  
- Unnecessary updates  
- Hardcoded logic & Data Duplication (DRY)
And prioritizes:
- Scalability  
- Maintainability  
- Performance  
---
## 📌 Notes
- This is a **technical showcase**, not a full gameplay feature
- Focus is on **architecture and code quality**
- Code is intentionally simplified for clarity and review
---
## 👨💻 Developer
**Mustafa Kum**  
Gameplay Programmer (C++ / Unreal Engine)
