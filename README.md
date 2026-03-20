# 🧠 Inventory System Showcase (Unreal Engine 5)

A modular, data-driven Inventory & Equipment System built in C++ with a focus on scalability, performance, and clean architecture.

---

## 🎯 Overview

This repository is a **code showcase** extracted from a larger RPG project.

It demonstrates how an inventory system can be designed for:
- Long-term scalability
- Clean architecture
- Efficient runtime behavior

The goal is not feature completeness, but **engineering quality**.

---

## 🧩 Architecture

The system is built around a few core principles:

- **Event-Driven Communication** (no polling)
- **Data-Driven Design** (no hardcoded values)
- **Separation of Responsibilities**
- **Minimal Runtime Overhead**

---

## ⚙️ Key Features

### 🔄 Event-Driven UI
- No `Tick` usage in UI
- Uses delegates (`OnInventoryUpdated`)
- UI updates only when state changes

---

### 📊 Data-Driven System
- All data defined via `UDataAsset`
- Weapons, stats, and UI data are fully configurable
- No logic-level hardcoding

---

### ⚡ Asynchronous Asset Loading
- Uses Unreal’s `StreamableManager`
- Loads assets only when needed
- Prevents game thread blocking

---

### 🧱 Modular & SOLID Structure
- High cohesion, low coupling
- Clear class responsibilities
- Fail-fast validation

---

### 🧬 Gameplay Ability System (GAS)
- Attribute Sets handle stat calculations
- Gameplay Tags define states (e.g. `State.Armed`)
- Scalable stat system

---

### 🖱️ Drag & Drop System
- Native Unreal drag & drop
- Payload-based operations
- Clear source/target separation

---

## 📂 Project Structure

All core files are placed in the root directory for quick review.

### Core Systems
- `InventoryComponent` → State & logic management  
- `CharacterBase` → Equipment & stat application  

### UI Layer
- `InventoryWidget` → UI orchestration  
- `WeaponSlotWidget` → Slot logic & visuals  
- `ItemTooltipWidget` → Dynamic item data  
- `StatRowWidget` → Reusable UI element  

### Data Layer
- `WeaponDataAsset` → Weapon definitions  
- `CharacterDataAsset` → Character config  

### Supporting Systems
- `AttributeSet` → GAS stat logic  
- `GameplayTags` → Global states  
- `DragDropOperation` → Interaction payload  

---

## 🧠 Design Philosophy

> Systems should remain stable as complexity grows.

This means:
- No hidden dependencies  
- No unnecessary updates  
- No tightly coupled logic  

---

## 📌 Notes

- This is a **technical showcase**, not a full feature
- Focus is on **architecture and code quality**
- Code is simplified where needed for clarity

---

## 👨‍💻 Developer

**Mustafa Kum**  
Gameplay Programmer (C++ / Unreal Engine)
