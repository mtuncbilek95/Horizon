<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://github.com/user-attachments/assets/ee3b6ea7-7056-4a5b-a8a8-6a361cbc1362">
    <source media="(prefers-color-scheme: light)" srcset="KOYU_LOGO_URL">
    <img width="600" alt="Horizon Engine" src="KOYU_LOGO_URL">
  </picture>
</p>

<div align="justify">

Horizon is a centralized and modularized game engine. It mainly targets desktop games via DirectX12. Overall flow is based on services which stay in the core application's
lifetime scope. Games run on a sparse-set entity component system, with gameplay written in C++ as game scripts and systems inside a hot-reloadable game module. Engine doesn't
have any goal or target. It is a passion project to make a game from scratch with a detailed custom game engine.

#### NOTE 1: PS5 and XBOX support has been marked as W.I.P and it will be private due to confidentiality contract.
#### NOTE 2: Don't go rogue. Check how to install section. Don't be so big-headed and read it.

## INFORMATION ABOUT THE ENGINE
Horizon has its own abstraction structures depending on the concept. Such as:
- RHI
- PAL
- IService (named as Service, inherits from Module)
- IContext (named as Context, inherits from Module)

Almost every abstraction has a clean custom implementation without depending on a third party library to make multiple structures user-friendly. Which means that PAL has a native
Win32API implementation. RHI is not nvrhi. Because `Why the hell not?`. It's a passion project so I can do whatever I want on the engine design. If anyone at some point wants to
use this engine, they will encounter a Unity blended with Unreal type of UI/UX. So, there is nothing to worry about for the user side.

### NO SMART POINTER
If you're a smart pointer + safe C++ person, please [check this article](https://www.youtube.com/watch?v=zWfxi0DrJjU).

### GRAPHICS
The base usage targets ReBAR with DirectX12 Ultimate. Which means that if the hardware doesn't support `ShaderModel 6.6`, user/consumer can run neither this engine nor the game
that has been made with it. Because `I don't care about backward compatibility. I have a Nvidia RTX3060 6GB Laptop and if it runs then I'm okay`.

### PLATFORM
Currently bound to Win11SDK and a couple of the Win11 features are mandatory for Horizon's window system. Because of the same reason at graphics section.

### REFLECTION
It's a label based runtime type reflection system. Every reflectable object has been labeled with `size_t` to prevent modifying project codes like Unreal does. This method comes
with benefits but that doesn't mean it is flawless. Data generation and data reflecting rules heavily follow C++ meta programming which means that accessing a type is fast as long
as the accessor stores the address of Engine::ReflectionSystem.

Here is an example how to use ReflectionSystem and Reflectables:
```cpp
void MyClass::Foo()
{
    auto* pReflection = GetEngine()->GetReflectionSystem();

    List<Reflect::Type*> types = pReflection->GetTypeByBase(Reflect::TypeOf<MyInterface>());
    for(auto* pType : types)
    {
        auto* pObject = (MyInterface*)pType->Create();
        if(!pObject)
        {
            Terminal::Error(StringOps::GetName(this), "{} probably has missing virtual function implementation.", pType->GetName());
            continue;
        }

        m_myObjectList.PushBack(pObject);
        Terminal::Info(StringOps::GetName(this), "{} has been registered successfully", pType->GetName());
    }
}
```

### CORE-ENGINE
Engine is a single struct that holds multiple services and contexts. Nothing happens inside of the engine except executing all services. The bigger question here is `What is service?`.
A service is basically an executable module `(on main thread)` that lives inside of the engine. A service can only be created inside of the engine, neither plugins nor the game project
should create and inject a service. Because `I don't want`. On the other hand, Context is just a bookkeeper module that doesn't have an execution phase. Same rules don't apply to
context as well.

`Just don't butcher the engine :(`

### WORLD SERVICE
The game is being processed here. Technically this place is the engine for most of you `(But it's not. I don't wanna explain, go educate yourself)`. The world consists of multiple systems
and the currently active scene. It has a custom sparse-set ECS `(Check out EnTT)`. Components are related to the scene but systems are related to the service due to planned features for the
plugin system.

### TBC...

## HOW TO RUN THE ENGINE
Horizon has multiple requirements:

1. Use Visual Studio 2026 (If possible because project heavily relies on CMakePresets.json)
2. You should have the latest Windows 11 SDK.
3. Don't generate a solution file. Trust CMake.
4. Don't run anything on terminal to build the project otherwise you will encounter lots of issues.

After the requirements, here is the list:

1. Open the project as folder via Visual Studio 2026.
2. Reconfigure CMake.
3. Build the project.
4. Run.
5. If still having issues, check the `NOTE 2` in the beginning.
6. This build system has been used in 8 different computers, so please shut up.

## MAKING A GAME

Currently, there are not that many features but here are some examples of what can be done. It should also be acknowledged that the style is pretty similar to the 2 commercial
game engines. So it will feel like using a baby of the two.

### GAME PROJECT
A game is a separate CMake project that builds into a DLL and gets loaded by the engine. `ExampleProject` is the reference:

```
ExampleProject/
├── Assets/
├── Code/                      <- every .h/.cpp in here is compiled automatically
├── Intermediate/              <- generated reflection data, don't touch
├── Binaries/                  <- the game DLL ends up here
├── Plugins/
├── CMakeLists.txt
├── CMakePresets.json
└── ExampleProject.hproject    <- tells the engine which DLL is the game module
```

The `CMakeLists.txt` is basically a one-liner:

```cmake
project("ExampleProject" LANGUAGES C CXX)

HorizonGameModule(${PROJECT_NAME}
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Code"
	INTERMEDIATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Intermediate"
	OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Binaries"
)
```

Things to know before you cry:

1. Build the engine **first**. The game project links against the engine build with the same preset name.
2. The engine preset must be a shared (non-monolithic) build, and its build type must match the game's (Debug with Debug, Release with Release).
3. Set `HORIZON_ROOT` in `CMakePresets.json` to your engine folder.
4. `<PROJECTNAME>_API` (e.g. `EXAMPLEPROJECT_API`) is generated from the project name. Put it on every class you write.
5. Hot-reload exists. Rebuild while the editor is running and the module gets reloaded. It works, but I'm sceptical as hell, so treat it as experimental.

### GAME SCRIPT OR COMPONENT + SYSTEM?
There are two ways to write gameplay. Pick by asking "how many of these are there?"

| | GameScript | Component + System |
|---|---|---|
| Feels like | Unity's MonoBehaviour | Unreal's Mass / any ECS |
| Use it for | One-off, entity-specific behaviour | Behaviour that exists on many entities |
| Example | Camera follow, the player | Doors, pickups, traps |
| Data and logic | Together in the script | Component = data only, System = logic only |

Components are **pure data**. If you put a function into a component, the system loses the reason to exist. Logic lives in the system's private functions.

Prefer composition over specialised components. Don't write `InteractableDoorComponent`. Write an `InteractableComponent` (range, prompt, "interacted this frame") and a
`DoorComponent` (angles, speed, state). Then a door that opens with a lever or a trigger doesn't need a copy-pasted door logic.

### NO REGISTRATION
There's no `RegisterSystem<T>()` or anything like that. Reflection finds every `GameScript` and `System` in your module through its base type.

The only catch is that **a system without a `SystemOrder` attribute does not get loaded**. The WorldService will tell you `You won't have this system.` and it means it.

### REFLECTION CHEATSHEET

#### Macros

| Macro | Where | What it does |
|---|---|---|
| `HCLASS(attrs...)` | Above a class | Marks the class for reflection, attributes go inside |
| `HORIZON_TYPE_REFLECT(T)` | Inside a class that inherits `Reflect::Base` | Gives the type its `GetTypeId()` and lets reflection reach private fields |
| `HORIZON_DATA_REFLECT(T)` | Inside a plain data struct | Same as above, without `GetTypeId()`. For settings/POD-like structs |
| `HORIZON_ATTRIBUTE_REFLECT(T)` | Inside an attribute class | Required when you write your own attribute |
| `HFIELD(attrs...)` | Above a member | Field is reflected, serialized and shown in the inspector |
| `HATTRIBUTE(attrs...)` | Anywhere inside a class body | Adds class attributes from inside the body instead of `HCLASS` |
| `HENUM()` | Above an enum | Reflects the enum. Bit-shifted flag enums are ignored on purpose |
| `HMETA("Name")` | Next to an enumerator | Display name of that enum value |

A member without `HFIELD` doesn't exist for reflection. Not saved, not shown, not your problem.

#### Attribute syntax
Attributes are written without the `Attribute` suffix, with arguments in square brackets. Separate multiple attributes with commas:

```cpp
HCLASS(Engine::ComponentId["DoorComponent", "Gameplay"]);

HFIELD(Range[0.f, 1.f], Slider, Tooltip["0 is closed, 1 is wide open"]);
f32 m_openAmount = 0.f;
```

Outside of the attribute's own namespace, qualify it (`Engine::ComponentId`, `Engine::SystemOrder`).

#### Writing your own attribute
An attribute is just a class. There's no registration, no magic, no macro soup beyond one line.

**1. Inherit from `Reflect::Attribute` and end the name with `Attribute`.**

```cpp
#pragma once

#include <Runtime/RTTR/Attribute.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::ExampleProject
{
	class GameplayTagAttribute final : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(GameplayTagAttribute);
	public:
		GameplayTagAttribute(const std::string& tag) : m_tag(tag)
		{
		}
		~GameplayTagAttribute() = default;

		const std::string& GetTag() const { return m_tag; }

	private:
		std::string m_tag;
	};
}
```

**2. Use it.** The square brackets are the constructor:

```cpp
#include "GameplayTagAttribute.h"

HCLASS(Engine::ComponentId["DoorComponent", "Gameplay"], GameplayTag["Interactable"]);
class EXAMPLEPROJECT_API DoorComponent final : public Engine::ComponentObject
```

**3. Read it.** Both `Reflect::Type` and `Reflect::Field` have `GetCustomAttribute<T>()` (returns `nullptr` if missing) and `GetCustomAttributes<T>()` (for stacked ones):

```cpp
b8 TagSystem::OnInitialize()
{
	auto* pReflection = GetEngine()->GetReflectionSystem();

	List<Reflect::Type*> types = pReflection->GetTypeByBase(Reflect::TypeOf<Engine::ComponentObject>());
	for (auto* pType : types)
	{
		auto* pTag = pType->GetCustomAttribute<GameplayTagAttribute>();
		if (!pTag)
			continue;

		Terminal::Info(StringOps::GetName(this), "{} is tagged as {}", pType->GetName(), pTag->GetTag());
	}

	return true;
}
```

How it works, so you don't fight it:

- The generator literally rewrites `GameplayTag["Interactable"]` into `GameplayTagAttribute("Interactable")`. Whatever is inside the brackets is pasted as C++, so `Reflect::TypeOf<T>()`, enums and `{ ".fbx", ".glb" }` all work.
- Multiple constructors are fine. `Step` and `Step[0.5f, 5.f]` are the same class.
- Writing the full name (`GameplayTagAttribute[...]`) also works, the suffix is only added when it's missing.
- The generated code includes **your** header, not the attribute's. So the header that uses the attribute must include it.
- The generated code lives in `Horizon::Reflect` with `using namespace` of your type's namespace. That's why `Range` and your own namespace's attributes work bare, but `Engine::ComponentId` needs the prefix.
- Attributes are header-only. They don't need `HCLASS` and don't need an `_API` macro.

#### All (Runtime) — usable everywhere, including game modules and plugins

| Attribute | Target | Usage | What it does |
|---|---|---|---|
| `Transient` | Field | `HFIELD(Transient)` | Reflectable, but never serialized. Lives and dies at runtime |
| `Alias` | Field | `HFIELD(Alias["m_oldName"])` | The field was renamed. Data saved under the former name still loads, both in editor metadata and cooked binaries. Can be stacked |
| `Blob` | Field | `HFIELD(Blob)` | Field is treated as a group of data. String fields get a multiline text box |
| `Range` | Field | `HFIELD(Range[min, max])` | Clamps numbers in the inspector. On strings, `max` is the character limit |
| `Slider` | Field | `HFIELD(Slider, Range[0, 1])` | Draws a slider instead of a drag box. Needs `Range` |
| `Step` | Field | `HFIELD(Step[0.5f, 5.f])` or `HFIELD(Step)` | Drag/slider step and fast step. Without arguments it's 1 and 10. On enums, turns the combo into a slider |
| `Degrees` | Field | `HFIELD(Degrees)` | Shown and edited as degrees |
| `ReadOnly` | Field | `HFIELD(ReadOnly)` | Shown in the inspector, can't be edited |
| `HideInInspector` | Field | `HFIELD(HideInInspector)` | Serialized, but not shown. The opposite of `Transient` |
| `Header` | Field | `HFIELD(Header["Movement"])` | Draws a section header above the field |
| `Tooltip` | Field | `HFIELD(Tooltip["Represents as degrees"])` | Hover text in the inspector |
| `AssetRef` | Field | `HFIELD(AssetRef[Reflect::TypeOf<MeshAsset>()])` | Asset picker that only accepts the given asset type |

`Transient` vs `HideInInspector`, because someone will ask:

| | Reflected | Serialized | In inspector |
|---|---|---|---|
| `HFIELD()` | yes | yes | yes |
| `HFIELD(Transient)` | yes | **no** | yes |
| `HFIELD(HideInInspector)` | yes | yes | **no** |
| no `HFIELD` | no | no | no |

#### Engine — usable in engine, game modules and plugins

| Attribute | Target | Usage | What it does |
|---|---|---|---|
| `ComponentId` | Component class | `HCLASS(Engine::ComponentId["DoorComponent", "Gameplay"])` | Name and category in the editor's Add Component menu |
| `SystemOrder` | System class | `HCLASS(Engine::SystemOrder[Engine::SystemPhase::Script + 500])` | Execution order, smaller runs first. **Mandatory**, a system without it is not loaded |

#### EditorOnly — only inside the Editor module

Game modules don't link against the Editor, so these are for engine/editor development only.

| Attribute | Target | Usage | What it does |
|---|---|---|---|
| `EditorView` | View class | `HCLASS(EditorView[ICON_FA_EYE, "Inspector", DockZone::Right, EditorViewFlags::OpenOnStart])` | Registers a dockable editor window: icon, title, dock zone, flags |
| `MainMenuItem` | Menu class | `HCLASS(MainMenuItem["File", 0])` | Top-level main menu entry and its order |
| `MenuItem` | Menu item class | `HCLASS(MenuItem["File/Save", 10, false])` | Entry under a main menu. The last argument makes it a checkbox |
| `ContextMenuItem` | Context menu class | `HCLASS(ContextMenuItem["AssetBrowserView", "Create/Folder", 0])` | Right-click entry: owner view, path, order |
| `ToolBarItem` | Toolbar class | `HCLASS(ToolBarItem[ToolBarSection::Center, 0])` | Toolbar button: section and order |
| `ImportType` | Importer class | `HCLASS(ImportType[Reflect::TypeOf<Engine::MeshAsset>(), { ".fbx" }])` | Which asset type this importer produces and from which extensions |
| `ImportPipelineInfo` | Import pipeline class | `HCLASS(ImportPipelineInfo[{ ".fbx", ".glb", ".gltf" }])` | Extensions this import pipeline handles |
| `ActionType` | Asset action class | `HCLASS(ActionType[Reflect::TypeOf<Engine::SceneAsset>()])` | Which asset type this action belongs to (e.g. double-click behaviour) |

#### Game Scripting Example
```cpp
#pragma once

#include <Engine/Game/GameScript.h>
#include <Engine/World/ECS/Definitions.h>
#include <Engine/World/Components/Physics/TransformComponent.h>
#include <Runtime/Math/Vec3f.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::ExampleProject
{
	HCLASS();
	class EXAMPLEPROJECT_API CameraFollowScript final : public Engine::GameScript
	{
		HORIZON_TYPE_REFLECT(CameraFollowScript);
	public:
		void OnStart() final;
		void OnUpdate(f32 deltaTime) final;
		void OnStop() final;

	private:
		Math::Vec3f RotateOffset(const Math::Vec3f& offset, f32 yaw) const;
		Math::Vec3f LookAtRotation(const Math::Vec3f& from, const Math::Vec3f& to) const;
		void FollowCamera(const Math::Vec3f& targetPosition, f32 targetYaw, Math::Vec3f& outPosition, Math::Vec3f& outRotation) const;

	private:
		HFIELD();
		Engine::EntityHandle m_target;

		HFIELD();
		f32 m_followSpeed = 2.0f;

		HFIELD();
		Math::Vec3f m_cameraOffset;

		HFIELD();
		Math::Vec3f m_lookOffset;

	private:
		Engine::TransformComponent* m_cameraTransform = nullptr;
		Engine::TransformComponent* m_targetTransform = nullptr;
	};
}
```

Grab what you need in `OnStart`, use it in `OnUpdate`. The helper functions are left out here:

```cpp
#include "CameraTrackerScript.h"

namespace Horizon::ExampleProject
{
	void CameraFollowScript::OnStart()
	{
		if (!m_target.IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "No assigned entity to target");
			return;
		}

		m_targetTransform = GetComponent<Engine::TransformComponent>(m_target);
		m_cameraTransform = GetComponent<Engine::TransformComponent>(GetEntity());
	}

	void CameraFollowScript::OnUpdate(f32 deltaTime)
	{
		if (!m_targetTransform || !m_cameraTransform)
			return;

		Math::Vec3f position;
		Math::Vec3f rotation;
		FollowCamera(m_targetTransform->m_position, m_targetTransform->m_rotation.Y(), position, rotation);

		const Math::Vec3f current = m_cameraTransform->m_position;
		const f32 t = Math::Min(1.0f, m_followSpeed * deltaTime);

		m_cameraTransform->m_position = current + (position - current) * t;
		m_cameraTransform->m_rotation = rotation;
	}

	void CameraFollowScript::OnStop()
	{
	}
}
```

#### Game Component and System Example
The component only holds data. `HFIELD`s are the designer's knobs, the rest is runtime state the system needs.

```cpp
#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/RTTR/Attributes/RangeAttribute.h>

namespace Horizon::ExampleProject
{
	HCLASS(Engine::ComponentId["DoorComponent", "Gameplay"]);
	class EXAMPLEPROJECT_API DoorComponent final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(DoorComponent);
	public:
		DoorComponent() = default;
		~DoorComponent() = default;

		HFIELD();
		b8 m_isOpen = false;

		HFIELD(Range[-180.f, 180.f]);
		f32 m_openAngle = 90.f;

		HFIELD(Range[0.f, 720.f]);
		f32 m_openSpeed = 180.f;

		f32 m_closedYaw = 0.f;
		f32 m_currentAngle = 0.f;
		b8 m_isInitialized = false;
	};
}
```

```cpp
#pragma once

#include <Engine/World/System.h>
#include <Engine/World/SystemOrderAttribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class TransformComponent;
}

namespace Horizon::ExampleProject
{
	class DoorComponent;

	HCLASS(Engine::SystemOrder[Engine::SystemPhase::Script + 500]);
	class EXAMPLEPROJECT_API DoorSystem final : public Engine::System
	{
		HORIZON_TYPE_REFLECT(DoorSystem);
	public:
		b8 OnInitialize() final;
		void OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene) final;
		void OnFinalize() final;
		void OnGroupsChanged(Engine::SystemGroup previous, Engine::SystemGroup current) final;

		Engine::SystemGroup GetWorkingGroup() const final { return Engine::SystemGroup::Script; }

	private:
		void InitializeDoor(Engine::TransformComponent& transform, DoorComponent& door);
		void AnimateDoor(Engine::TransformComponent& transform, DoorComponent& door, f32 deltaTime);
	};
}
```

The system walks every entity that has both a transform and a door. That's the whole trick:

```cpp
#include "DoorSystem.h"
#include "DoorComponent.h"

#include <Engine/World/Components/Physics/TransformComponent.h>

#include <cmath>

namespace Horizon::ExampleProject
{
	b8 DoorSystem::OnInitialize()
	{
		return true;
	}

	void DoorSystem::OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene)
	{
		currentScene.ForEach<Engine::TransformComponent, DoorComponent>([&](Engine::EntityHandle entity, Engine::TransformComponent& transform, DoorComponent& door)
		{
			if (!door.m_isInitialized)
				InitializeDoor(transform, door);

			AnimateDoor(transform, door, ctx.DeltaTime());
		});
	}

	void DoorSystem::OnFinalize()
	{
	}

	void DoorSystem::OnGroupsChanged(Engine::SystemGroup previous, Engine::SystemGroup current)
	{
	}

	void DoorSystem::InitializeDoor(Engine::TransformComponent& transform, DoorComponent& door)
	{
		door.m_closedYaw = transform.m_rotation.Y();
		door.m_currentAngle = 0.f;
		door.m_isInitialized = true;
	}

	void DoorSystem::AnimateDoor(Engine::TransformComponent& transform, DoorComponent& door, f32 deltaTime)
	{
		const f32 target = door.m_isOpen ? door.m_openAngle : 0.f;
		const f32 difference = target - door.m_currentAngle;
		const f32 step = door.m_openSpeed * deltaTime;

		if (std::abs(difference) <= step)
			door.m_currentAngle = target;
		else
			door.m_currentAngle += difference > 0.f ? step : -step;

		const Math::Vec3f rotation = transform.m_rotation;
		transform.m_rotation = Math::Vec3f(rotation.X(), door.m_closedYaw + door.m_currentAngle, rotation.Z());
	}
}
```

### SYSTEM ORDER AND GROUPS
Systems run in ascending `SystemOrder`. The rule is simple: **round thousands belong to the engine, the gaps belong to you.**

| Order | Owner | System |
|---|---|---|
| 0 - 999 | Game / Plugin | Before everything (input, interaction detection) |
| 1000 | Engine | `SystemPhase::Script` (ScriptSystem, runs GameScripts) |
| 1001 - 1999 | Game / Plugin | After scripts, before physics |
| 2000 | Engine | `SystemPhase::Physics` |
| 3000 | Engine | `SystemPhase::Transform` |
| 4000 | Engine | `SystemPhase::Camera` |
| 9000 | Engine | `SystemPhase::Mesh` |
| u32_max | Engine | RenderSystem |

So `SystemPhase::Script + 500` means "after scripts, before physics". A GameScript can set `m_isOpen = true` and the door starts moving in the same frame,
and the transform is written before physics reads it. Don't take a round thousand. It's mine.

`GetWorkingGroup()` decides in which world state the system runs:

| Group | Runs in |
|---|---|
| `General`, `Render` | Edit, Play, Pause |
| `Script`, `Audio`, `Physics` | Play only |
| `Editor` | Edit only |

Gameplay goes to `Script`, unless you enjoy doors flapping around while you're placing them in the editor.

### TBC...

</div>