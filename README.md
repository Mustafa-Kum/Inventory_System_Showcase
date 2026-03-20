# Inventory System Showcase (Unreal Engine 5)

> **Note:** This repository is a **C++ Code Showcase** and a portfolio piece extracted from a custom my RPG project. All core files have been placed directly in the root directory for easy, immediate code review.

## 📌 Architectural Overview

This project demonstrates a highly modular, scalable, and data-driven **Inventory and Equipment System**. It was built to avoid tightly-coupled code and heavy CPU usage, prioritizing **SOLID principles**, an **Event-Driven UI**, **Asynchronous Asset Loading**, and integration with Unreal's **Gameplay Ability System (GAS)**.

---

## 🏗️ Key Engineering Features

1. **Event-Driven UI (Observer Pattern):** UI Widgets avoid `NativeTick()`. The [InventoryComponent](cci:1://file:///d:/WoWClone/Source/WoWClone/Private/Components/InventoryComponent.cpp:9:0-12:1) broadcasts an `OnInventoryUpdated` delegate, and the UI rebuilds instantly and **only** when a state changes (e.g., looting or equipping).
2. **Data-Driven Design (Single Source of Truth):** No hardcoded stats. Every weapon and character base stat is driven dynamically via `UDataAsset` (such as `WeaponDataAsset` and `CharacterDataAsset`).
3. **Asynchronous Asset Loading:** 3D Meshes and UI Icons are loaded asynchronously into memory using Unreal's `StreamableManager` only when needed, ensuring the Game Thread never hitches.
4. **SOLID Methods:** Logic flows through clear, fail-fast validation (e.g., [CanEquipWeapon()](cci:1://file:///d:/WoWClone/Source/WoWClone/Private/Components/InventoryComponent.cpp:287:0-290:1) ➡️ `InternalProcessEquipFlow()` ➡️ [ExecuteVisualTransition()](cci:1://file:///d:/WoWClone/Source/WoWClone/Private/Components/InventoryComponent.cpp:207:0-211:1)) without deep conditional nesting.
5. **Gameplay Ability System (GAS):** Stats are safely manipulated mathematically through [UCharacterAttributeSet](cci:1://file:///d:/WoWClone/Source/WoWClone/Private/Abilities/AttributeSets/CharacterAttributeSet.cpp:5:0-10:1), and conditions like `State.Armed` are managed deterministically via Gameplay Tags.
6. **Native Drag & Drop:** Uses native payload operations (`UWeaponDragDropOperation`) to seamlessly detect Drop Source and Drop Target contexts, routing to the correct logic handler securely.

---

## 📂 Included C++ Classes (Root Directory)

* **[InventoryComponent](cci:1://file:///d:/WoWClone/Source/WoWClone/Private/Components/InventoryComponent.cpp:9:0-12:1) (.h/.cpp):** The core managing backend logic, state machines, and delegate broadcaster.
* **[CharacterBase](cci:1://file:///d:/WoWClone/Source/WoWClone/Private/Characters/CharacterBase.cpp:10:0-29:1) (.h/.cpp):** The Pawn. Handles physical mesh attachments and GAS attribute application securely.
* **[InventoryWidget](cci:1://file:///d:/WoWClone/Source/WoWClone/Private/Characters/HeroCharacter.cpp:326:0-340:1) (.h/.cpp):** The frontend orchestrator. Builds the grid and sub-widgets.
* **`WeaponSlotWidget` (.h/.cpp):** Individual grid slots managing Async Icon loading and Drag/Drop native callbacks.
* **[ItemTooltipWidget](cci:1://file:///d:/WoWClone/Source/WoWClone/Private/UI/ItemTooltipWidget.cpp:9:0-21:1) (.h/.cpp):** Dynamically updates text and UI based strictly on the injected `WeaponDataAsset`.
* **`StatRowWidget` (.h/.cpp):** Dynamic row generator inside the Tooltip ensuring Single Responsibility.
* **`WeaponDragDropOperation` (.h/.cpp):** Payload carrier for drag-and-drop actions.
* **`WeaponDataAsset` (.h/.cpp):** The core weapon template carrying soft-references to heavy assets and base stats.
* **`CharacterDataAsset` (.h/.cpp):** The core player configuration template.
* **[CharacterAttributeSet](cci:1://file:///d:/WoWClone/Source/WoWClone/Private/Abilities/AttributeSets/CharacterAttributeSet.cpp:5:0-10:1) (.h/.cpp):** The GAS attribute set defining logic like Agility automatically increasing Weapon Haste.
* **`WoWCloneGameplayTags` (.h/.cpp):** Global state definitions.

---
*Developed by [Mustafa Kum]*
