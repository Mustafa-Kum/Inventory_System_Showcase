# 🧠 Inventory System Showcase (Unreal Engine 5)

![Unreal Engine](https://img.shields.io/badge/Unreal_Engine-5.0%2B-blue?logo=unrealengine&logoColor=white)
![C++](https://img.shields.io/badge/C%2B%2B-High_Performance-blue)
![Architecture](https://img.shields.io/badge/Architecture-Modular-green)
![System](https://img.shields.io/badge/System-Data--Driven-orange)

---

<div align="center">
  <img src="Gifs/ItemDetail.gif" width="32%">
  <img src="Gifs/ItemEquip.gif" width="32%">
  <img src="Gifs/ItemUnequip.gif" width="32%">
  <p><em>Event-driven UI updates, drag & drop interactions, and dynamic tooltip system.</em></p>
</div>

---

## 📖 Overview

This project is a **C++ code showcase** demonstrating a modular and scalable **Inventory & Equipment System** built for Unreal Engine 5.

The focus is on **architecture and engineering quality**, not feature completeness.

The system is designed to:
- Scale with increasing complexity  
- Remain easy to maintain  
- Avoid unnecessary runtime cost  

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
- No `Tick` usage in UI
- Delegate-based updates (`OnInventoryUpdated`)
- UI rebuilds only when necessary

---

### 📊 Data-Driven System
- All data defined via `UDataAsset`
- Weapons, stats, and UI driven entirely by data
- Fully designer-friendly structure

---

### ⚡ Asynchronous Asset Loading
- Uses Unreal’s `StreamableManager`
- Loads meshes and icons on demand
- Prevents frame hitches

---

### 🧱 Modular & SOLID Architecture
- High cohesion, low coupling
- Clear separation between systems
- Fail-fast validation patterns

---

### 🧬 Gameplay Ability System (GAS)
- Attribute Sets handle stat calculations
- Gameplay Tags define states (e.g. `State.Armed`)
- Clean and extensible stat pipeline

---

### 🖱️ Drag & Drop System
- Native Unreal drag & drop
- Payload-based operations
- Clear source/target handling

---

## 📂 Project Structure

All core files are placed in the root directory for fast review.

### Core Systems
- `InventoryComponent` → State & logic management  
- `CharacterBase` → Equipment handling & stat application  

### UI Layer
- `InventoryWidget` → UI orchestration  
- `WeaponSlotWidget` → Slot behavior & async icon loading  
- `ItemTooltipWidget` → Dynamic item details  
- `StatRowWidget` → Reusable UI element  

### Data Layer
- `WeaponDataAsset` → Weapon definitions  
- `CharacterDataAsset` → Character configuration  

### Supporting Systems
- `AttributeSet` → GAS stat calculations  
- `GameplayTags` → Global state definitions  
- `DragDropOperation` → Interaction payload  

---

## 🧠 How It Works

1. Player interacts with inventory (drag/drop, equip, inspect)
2. Inventory Component updates internal state
3. Delegate (`OnInventoryUpdated`) is broadcast
4. UI listens and rebuilds only affected elements
5. Assets are loaded asynchronously when required
6. GAS updates character stats and state via tags

---

## 🧠 Design Philosophy

> Systems should remain stable as complexity grows.

This system avoids:
- Hidden dependencies  
- Unnecessary updates  
- Hardcoded logic  

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

## 👨‍💻 Developer

**Mustafa Kum**  
Gameplay Programmer (C++ / Unreal Engine)
