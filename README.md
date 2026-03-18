# Horizon Engine

A multi-platform (Linux/Windows) game engine built on a modular system architecture, Vulkan RHI, and EnTT-based ECS.


## Important Note

I can't call this a portfolio project in the traditional sense, but there are those who accept working with a mix of vibe coding in today's landscape (I absolutely don't use it in corporate R&D projects, but it proves effective in public hobby projects like this). Sometimes I work with vibe coding, sometimes fully handwritten. I use vibe coding mostly to speed up parts I expect to take a very long time to write. The system architecture design is entirely my own. The primary goal of this project is to build a fully automated engine system and use it as a workbench for studying different rendering research papers. If you want to evaluate it as a portfolio piece, focus on the architectural design.


## Architecture Overview

The engine is organized into two layers:

- **Runtime** — platform primitives: windowing, input, Vulkan wrappers, logging, math, job primitives
- **Engine** — high-level systems built on top of Runtime

All high-level functionality lives in systems. Every system follows the same lifecycle and communicates through a central `Engine` object.


## System Lifecycle

Every system derives from `System<T>` (CRTP) and implements three methods:

```
OnInitialize()  →  OnSync() [every frame]  →  OnFinalize()
```

Systems are registered at startup via `engine.AddSystem<T>()`. The engine processes them in order:

1. Initialize pending systems
2. Finalize pending systems
3. Sync all active systems

Inter-system communication happens through `RequestSystem<U>()`, available to any system.


## Initialization Order & Dependencies

```
WindowSystem
  └─ creates BasicWindow (1920×1080), holds InputDispatcher

VirtualFileSystem
  └─ mounts  Assets://  →  ../assets
              Shaders:// →  ../shaders

JobSystem
  └─ spawns (CPU count - 1) worker threads

AssetSystem
  ├─ scans Shaders://**  →  ShaderAsset  (SPIR-V compilation, parallel via JobSystem)
  └─ scans Assets://*.hscene  →  SceneAsset  (JSON parse, parallel via JobSystem)

GraphicsSystem
  └─ creates GfxInstance + GfxDevice (Graphics / Compute / Transfer queues)

PresentationSystem
  └─ creates swapchain  (double-buffered, platform-specific format)

InputSystem
  └─ gets InputDispatcher from WindowSystem
  └─ RegisterInput(InputType, callback) routes to dispatcher

EntityComponentSystem
  ├─ AddSystem<CameraSystem>()
  └─ AddSystem<MeshRendererSystem>()

GameSystem
  └─ OnInitialize sets GameState → Start
```

## Game State Machine

`GameSystem` holds the current `GameState`. `EntityComponentSystem` reads it every sync:

```
Idle ──► Start ──► Run ◄──► Pause
                    │
                    ▼
                   Stop ──► Idle
```

| State  | ECS action |
|--------|-----------|
| Start  | Calls `OnStart()` on all EntitySystems, then transitions to Run |
| Run    | Calls `OnTick()` on all EntitySystems every frame |
| Pause  | No action |
| Stop   | Calls `OnStop()` on all EntitySystems, then transitions to Idle |


## Asset System

Assets are stored by virtual path in `AssetSystem`. Retrieval is type-safe:

```cpp
auto* scene = assetSystem.GetAsset<SceneAsset>("Assets://Levels/Level1.hscene");
```

### SceneAsset

Scene files use the `.hscene` extension and are plain JSON.

```json
{
  "entities": [
    {
      "transform": { "position": [0,0,0], "rotation": [1,0,0,0], "scale": [1,1,1] },
      "mesh": { "meshPath": "Assets://Meshes/Cube.mesh", "materialPath": "Assets://Materials/Default.mat" }
    },
    {
      "transform": { "position": [0,5,-10], "rotation": [1,0,0,0], "scale": [1,1,1] },
      "camera": { "fov": 60.0, "near": 0.1, "far": 1000.0 }
    }
  ]
}
```

**Import flow:**
```
SceneAsset::Import()       — parses JSON, caches in m_parsedJson
SceneAsset::ImportInto(registry)  — creates entt entities and emplaces components
SceneAsset::ExportFrom(registry)  — serializes registry back to JSON file
```

Serialization is driven by `SceneComponents` tuple and `ComponentTraits`. Adding a new component requires three steps:

1. Write the component struct + `from_json` / `to_json` in `Components/`
2. Add a `ComponentTraits` specialization with the JSON key
3. Add the type to `SceneAsset::SceneComponents`

No other files need to be touched.

## Entity Component System

`EntityComponentSystem` manages a set of `EntitySystem` sub-systems (e.g. `CameraSystem`, `MeshRendererSystem`) and an `entt::registry` per scene.

### Components

| Component | JSON key | Fields |
|-----------|----------|--------|
| `TransformComponent` | `"transform"` | position (Vec3f), rotation (Quatf), scale (Vec3f) |
| `MeshComponent` | `"mesh"` | meshPath, materialPath |
| `CameraComponent` | `"camera"` | fov, nearPlane, farPlane |

### EntitySystem sub-systems

Sub-systems derive from `EntitySystem` and implement:

```cpp
void OnStart();   // called once when GameState transitions to Start
void OnTick();    // called every frame while GameState is Run
void OnStop();    // called once when GameState transitions to Stop
```

They can access any Engine-level system via `RequestSystem<T>()`.

## Job System

`JobSystem` provides a dependency-aware thread pool.

```cpp
JobHandle h1 = jobs.Submit([] { /* task A */ });
JobHandle h2 = jobs.Submit([] { /* task B, runs after A */ }, { h1 });
```

- Worker count: `hardware_concurrency - 1`
- Jobs with unmet dependencies sit in a pending queue and are promoted automatically when their dependencies complete.

---

## Virtual File System

All file paths go through `VirtualFileSystem` using `mountName://path` syntax.

```cpp
vfs.Resolve("Assets://Levels/Level1.hscene");  // → physical path
vfs.ScanFiles("Assets://", ".hscene");          // → all .hscene files
```

Mount points are registered at startup and can be added or removed at runtime.


## Input

```cpp
inputSystem.RegisterInput(InputType::Key, [](const InputMessage& msg) {
    if (msg.key == KeyCode::W && msg.keyAction == InputAction::Press) { ... }
});

inputSystem.RegisterInput(InputType::MouseButton, [](const InputMessage& msg) { ... });
```

Supported types: `Key`, `MouseButton`, `MouseMove`, `MouseScroll`, `Char`, `Resize`.
