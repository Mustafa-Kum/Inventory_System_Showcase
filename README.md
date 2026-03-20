# Inventory System Showcase (Unreal Engine 5)

> **Note:** This repository is a **C++ Code Showcase** and a portfolio piece extracted from a my custom RPG project. All core files have been placed directly in the root directory for easy, immediate code review.

## 📌 Architectural Overview

This project demonstrates a highly modular, scalable, and data-driven **Inventory and Equipment System**. It was built to avoid tightly-coupled code and heavy CPU usage, prioritizing **SOLID principles**, an **Event-Driven UI**, **Asynchronous Asset Loading**, and integration with Unreal's **Gameplay Ability System (GAS)**.

---

## 🏗️ Key Engineering Features

1. **Event-Driven UI (Observer Pattern):** UI Widgets avoid NativeTick(). The inventory component broadcasts an OnInventoryUpdated delegate, and the UI rebuilds instantly and **only** when a state changes (e.g., looting or equipping).
2. **Data-Driven Design (Single Source of Truth):** No hardcoded stats. Every weapon and character base stat is driven dynamically via UDataAsset (such as Weapon Data Asset and Character Data Asset).
3. **Asynchronous Asset Loading:** 3D Meshes and UI Icons are loaded asynchronously into memory using Unreal's StreamableManager only when needed, ensuring the Game Thread never hitches.
4. **SOLID Methods:** Logic flows through clear, fail-fast validation without deep conditional nesting.
5. **Gameplay Ability System (GAS):** Stats are safely manipulated mathematically through Attribute Sets, and conditions like State.Armed are managed deterministically via Gameplay Tags.
6. **Native Drag & Drop:** Uses native payload operations to seamlessly detect Drop Source and Drop Target contexts, routing to the correct logic handler securely.

---

## 📂 Included C++ Classes (Root Directory)

* **Inventory Component (.h/.cpp):** The core managing backend logic, state machines, and delegate broadcaster.
* **Character Base (.h/.cpp):** The Pawn. Handles physical mesh attachments and GAS attribute application securely.
* **Inventory Widget (.h/.cpp):** The frontend orchestrator. Builds the grid and sub-widgets.
* **Weapon Slot Widget (.h/.cpp):** Individual grid slots managing Async Icon loading and Drag/Drop native callbacks.
* **Item Tooltip Widget (.h/.cpp):** Dynamically updates text and UI based strictly on the injected DataAsset.
* **Stat Row Widget (.h/.cpp):** Dynamic row generator inside the Tooltip ensuring Single Responsibility.
* **Weapon Drag Drop Operation (.h/.cpp):** Payload carrier for drag-and-drop actions.
* **Weapon Data Asset (.h/.cpp):** The core weapon template carrying soft-references to heavy assets and base stats.
* **Character Data Asset (.h/.cpp):** The core player configuration template.
* **Character Attribute Set (.h/.cpp):** The GAS attribute set defining logic like Agility automatically increasing Weapon Haste.
* **WoWClone Gameplay Tags (.h/.cpp):** Global state definitions.

---
*Developed by [Adınız Soyadınız]*
