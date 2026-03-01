# Terminus (HorseMenu) - Agent Onboarding & Codebase Documentation

Welcome to the Terminus (formerly HorseMenu) project! This document serves as a comprehensive guide for AI agents and human developers to understand the project's architecture, key components, and the standard workflow for contributing.

## Project Overview
Terminus is a beta-stage mod menu for Red Dead Redemption 2 (RDR2) and Red Dead Online, inspired by YimMenu. It is injected into `rdr2.exe` to provide various features, protect against crashes, and enhance the game experience.

**Core Technologies:**
- **C++23**: The standard used for the project.
- **CMake**: Build system (`CMakeLists.txt`).
- **ImGui**: For rendering the graphical user interface.
- **Vulkan**: The graphics API used for the overlay.
- **MinHook**: Library used for API hooking and intercepting game functions.

## Directory Structure & Components

The source code is primarily located in the `src/` directory, divided into three main pillars:

### 1. `src/core/` (The Engine)
This directory contains essential, general-purpose features that form the base of the mod menu. These components are generally **not** game-specific.
- **`backend/`**: Core backend logic and state management.
- **`commands/`**: Command system architecture (base classes for executing actions).
- **`filemgr/`**: File management utilities (handling config files, caches).
- **`frontend/`**: ImGui base setup, window management, and UI styling.
- **`hooking/`**: The hooking engine wrapping MinHook.
- **`logger/`**: Asynchronous logging system.
- **`memory/`**: Memory manipulation utilities, including pattern scanning, patchers, and memory allocation handling.
- **`renderer/`**: Vulkan renderer setup and ImGui integration.
- **`settings/`**: Configuration and settings management.

### 2. `src/game/` (The Game Implementation)
This directory contains everything specific to Red Dead Redemption 2.
- **`features/`**: The actual mod menu features, which are implemented as subclasses of `Command` (e.g., `LoopedCommand`, `BoolCommand`). These are organized into several subcategories:
  - `self/`: Player-specific features (Godmode, Noclip, Infinite Ammo).
  - `network/`: Online and session-specific features.
  - `players/`: Features affecting other players in the session.
  - `mount/` & `vehicle/`: Features for horses, wagons, and other vehicles.
  - `world/`: Environmental features (time, weather, gravity).
  - `spoofing/`: Identity protection features.
  - `system/`: Engine/system-level game behaviors.
- **`frontend/`**: The UI menus and tabs. This includes submenus (`src/game/frontend/submenus/`) where features are added to the ImGui GUI using their JOAAT hashed names (e.g., `std::make_shared<BoolCommandItem>("godmode"_J)`).
- **`hooks/`**: Game-specific function hooks (intercepting RDR2 engine calls).
- **`pointers/`**: Definitions and resolution of pointers to game structures and functions via memory patterns.
- **`rdr/`**: RDR2 specific classes, structures, and native definitions.
- **`backend/` & `bigfeatures/`**: Complex game-specific logic that spans multiple features.

### 3. `src/util/` (Utilities)
Loose helper functions and utilities that support both core and game-specific code.
- **Math & Hashing**: `Math.hpp`, `Joaat.hpp` (Jenkins One-at-a-Time hashing used heavily by Rockstar).
- **Game Utilities**: `SpawnObject.hpp`, `teleport.hpp`, `VehicleSpawner.hpp`, `Rewards`.
- **Serialization**: `Protobufs`.
- **Misc**: String to Hex conversions, Chat utilities.

## Agent Onboarding Instructions

When working on Terminus, agents must follow these guidelines:

### 1. Adding a New Feature
1. **Logic Implementation**: Create a new `.cpp` file in the appropriate `src/game/features/<category>/` folder. Inherit from a base command class found in `src/core/commands/` (e.g., `LoopedCommand`, `BoolCommand`). Override `OnTick()` or `OnEnable()`/`OnDisable()` and statically instantiate your command with its parameters (name, label, description).
2. **UI Integration**: Open the corresponding submenu file in `src/game/frontend/submenus/` (e.g., `Self.cpp` for a `self` feature). Add your command to a group using `AddItem(std::make_shared<CommandItem>("yourcommandname"_J));`. Note the use of the `_J` suffix for JOAAT hashing.
3. **Pointers**: If your feature requires calling a game internal function or accessing a structure not yet defined, find the pattern/offset and add it to `src/game/pointers/`.

### 2. Adding a Hook
1. **Definition**: Define your hook in `src/game/hooks/`.
2. **Registration**: Ensure the hook is properly registered with the core hooking engine (`src/core/hooking/`).
3. **Safety**: Always make sure to call the original function (trampoline) unless you specifically intend to block or completely override the game's behavior.

### 3. Updating Documentation (Crucial for Agents)
- **Continuous Documentation**: When you add a new major feature, structural pattern, or significant hook, you **MUST** update `AGENTS.md` (or create a specific `.md` doc in a `docs/` folder) to reflect these architectural changes.
- **Explain the "Why"**: Don't just list what was added. Explain *how* it fits into the broader architecture (e.g. "We introduced a new `Event` system for network packets, located in `src/core/network/`").
- Maintain this living document so future AI agents (and human developers) have the latest context.

### 4. Code Style & Best Practices
- **Logging**: Use the existing logging system (`src/core/logger/`) for debug and error messages instead of `std::cout`.
- **Modern C++**: Leverage C++23 features where appropriate for safety and performance (e.g., `std::span`, `std::expected`, smart pointers).
- **Memory Safety**: Be extremely careful when reading/writing game memory. Validate pointers before dereferencing them.
- **Build System**: `CMakeLists.txt` uses `file(GLOB_RECURSE SRC_FILES ...)` for the `src/` directory. When creating new files, you typically do not need to manually add them to CMake, just ensure they are in the correct subdirectory inside `src/` and CMake will pick them up upon regeneration.

### 5. Exploring the Codebase & Modding Resources
- If you need a specific RDR2 native or class, check `src/game/rdr/` before assuming it doesn't exist.
- Use `src/util/Joaat.hpp` for generating hashes, as RDR2 relies heavily on JOAAT hashes for models, weapons, etc.
- **External Resources**: When working on modding the game, finding natives, models, or discovering internal structures, heavily utilize these resources:
  - [femga/rdr3_discoveries](https://github.com/femga/rdr3_discoveries) - Excellent repository for game structures, datasets, and discoveries.
  - [rdr3natives.com](https://rdr3natives.com/) - Searchable database of RDR2 natives.
  - [redlookup.com/natives](https://redlookup.com/natives/) - Another great native reference.

## Final Notes for Agents
Before modifying existing code, always use search tools like `grep_search` to understand where a function or variable is used. The separation between `core` (engine) and `game` (RDR2) must be strictly maintained to ensure the codebase remains clean and potentially portable to other Rockstar titles in the future.
