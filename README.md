<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://github.com/user-attachments/assets/ee3b6ea7-7056-4a5b-a8a8-6a361cbc1362">
    <source media="(prefers-color-scheme: light)" srcset="KOYU_LOGO_URL">
    <img width="600" alt="Horizon Engine" src="KOYU_LOGO_URL">
  </picture>
</p>

<div align="justify">

Horizon is a centeralized and modularized game engine. It mainly targets desktop games via DirectX12. Overall flow is based on services which stay in core application's 
life time scope. Games run on sparse-set entity component system with singular lua scripting per entity. Engine doesn't have any goal or target. It is a passion project to 
make a game from scratch with a detailed custom game engine. 

#### NOTE 1: PS5 and XBOX support has been marked as W.I.P and it will be private due to confidentiality contract.
#### NOTE 2: Don't go rogue. Check how to install section. Don't be so big-headed and read it.

## INFORMATION ABOUT THE ENGINE
Horizon has it's own abstraction structures depending on the concept. Such as:
- RHI
- PAL
- IService (named as Service, inherits from Module)
- IContext (named as Context, inherits from Module)

Almost every abstraction has a clean custom implementation without depending on a third party library to make multiple structure user-friendly. Which means that PAL has a native
Win32API implementation. RHI is not nvrhi. Because `Why the hell not?`. It's a passion project so I can do whatever I want on the engine design. If anyone at somepoint wants to
use this engine, they will encounter with a Unity blended with Unreal type of UI/UX. So, there is nothing to worry about for the user side.

### NO SMART POINTER
If you're a smart pointer + safe C++ person, please [check this article](https://www.youtube.com/watch?v=zWfxi0DrJjU).  

### GRAPHICS
The base usage targets ReBAR with DirectX12 Ultimate. Which means that If the hardware doesnt support `ShaderModel 6.6`, user/consumer cannot run nor this engine or the game that
has been made with it. Because `I don't care about backward compatibility. I have a Nvidia RTX3060 6GB Laptop and if it runs then I'm okay`.

### PLATFORM
Currently bound to Win11SDK and couple of the Win11 features are mandatory for Horizon's window system. Because the same reason at graphics section.

### REFLECTION
It's an label based runtime type reflection system. Every reflectable object has been labeled with `size_t` to prevent modifying project codes like Unreal does. This method comes
with benefits but that doesn't mean it is flawless. Data generation and data reflecting rules heavily follows C++ meta programming which means that accessing a type is fast as long
as accessor stores the address of Engine::ReflectionSystem. 

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
A service is basically an executable module `(on main thread)` that lives inside of the engine. A service only can be created inside of the engine, none of the plugins or game project
should not create and inject a service. Because `I don't want`. On the other hand, Context is just a bookkeeper module that doesn't have execution phase. No same rules for context as 
well. 

`Just don't butcher the engine :(`

### WORLD SERVICE
The game is being processed here. Technically this place is the engine for most of you `(But it's not. I don't wanna explain, go educate yourself)`. The world consists of multiple systems 
and the currently active scene. It has custom sparse-set ECS `(Check out EnTT)`. Components are related with scene but systems are related with the service due to planned features for the 
plugin system. 

### TBC...

## HOW TO RUN THE ENGINE
Horizon has multiple requirements:

1. Use Visual Studio 2026 (If possible because project heavily relies on CMakePresets.json)
2. You should have latest Windows 11 SDK.
3. Don't generate a solution file. Trust CMake.
4. Don't run anything on terminal to build the project otherwise you will encounter with lots of issues.

After the requirements, here is the list:

1. Open the project as folder via Visual Studio 2026.
2. Reconfigure CMake.
3. Build the project.
4. Run.
5. If still having issue, check the `NOTE 2` in the beginning.
6. This build system has been used in 8 different computers, so please shut up.

## SCRIPTING EXAMPLE

This script example will be fixed later but for now it will help you understand what to do.

```lua
local Showcase = {}

Showcase.Properties = {
	enabled = true,
	maxBounces = 3,
	bobHeight = 0.25,
	spinSpeed = Range(45.0, 0.0, 360.0),
	bobSpeed = Range(2.0, 0.1, 10.0),

	greeting = "Hello from Lua",

	uvScroll = Vector2(0.5, 0.0),
	spinAxis = Vector3(0, 1, 0),
	bobOffset = Vector3(0, 1, 0),
	baseOrientation = Quaternion(0, 0, 0, 1),

	lookTarget = Entity(),
	alternateMesh = Asset("MeshAsset"),

	elapsedTime = ReadOnly(0.0),
	frameCount = ReadOnly(0),
	currentHeight = ReadOnly(Range(0.0, -10.0, 10.0)),
	lastPosition = ReadOnly(Vector3(0, 0, 0)),
	status = ReadOnly("Not started"),
}

local TwoPi = math.pi * 2.0

local function Describe(vector)
	return string.format("(%.2f, %.2f, %.2f)", vector.X, vector.Y, vector.Z)
end

function Showcase:OnStart()
	print(self.greeting, "| enabled:", self.enabled, "| bounces:", self.maxBounces)
	print("Owner entity valid:", self.entity:IsValid())

	self.transform = self.entity:Get("TransformComponent")

	if self.transform == nil then
		self.status = "No TransformComponent"
		error("PropertiesShowcase needs a TransformComponent")
	end

	self.startPosition = self.transform.Position
	self.startRotation = self.transform.Rotation

	print("Start position:", Describe(self.startPosition))

	if self.entity:Has("MeshComponent") then
		self.mesh = self.entity:Get("MeshComponent")
		print("Mesh hidden in render:", self.mesh.HideInRender)
	end

	if self.lookTarget:IsValid() then
		local targetTransform = self.lookTarget:Get("TransformComponent")

		if targetTransform ~= nil then
			print("Look target is at:", Describe(targetTransform.Position))
		end
	else
		print("No look target assigned; drag one from the hierarchy")
	end

	if self.lookTarget == self.entity then
		print("Look target is ourselves")
	end

	if self.alternateMesh ~= nil then
		print("Alternate mesh assigned, valid:", self.alternateMesh:IsValid())
	else
		print("No alternate mesh assigned")
	end

	if self.entity:Has("CameraComponent") then
		local camera = self.entity:Get("CameraComponent")
		print("Camera target screen:", camera.TargetScreen.X, camera.TargetScreen.Y)

		local ok, message = pcall(function()
			camera.TargetScreen = Vector2(1, 1)
		end)

		print("Writing a read-only field succeeded?", ok, message)
	end

	local ok, message = pcall(function()
		return self.transform.Postion
	end)
	print("Typo in a field name:", ok, message)

	ok, message = pcall(function()
		self.transform.Position = "not a vector"
	end)
	print("Wrong value type:", ok, message)

	ok, message = pcall(function()
		return self.entity:Get("NotARealComponent")
	end)
	print("Unknown component type:", ok, message)

	self.status = "Running"
end

function Showcase:OnUpdate(dt)
	self.frameCount = self.frameCount + 1
	self.elapsedTime = self.elapsedTime + dt

	if not self.enabled then
		self.status = "Paused by 'enabled' checkbox"
		return
	end

	self.status = "Running"

	local bob = math.sin(self.elapsedTime * self.bobSpeed * TwoPi) * self.bobHeight
	local offset = self.bobOffset:Normalized() * bob

	self.currentHeight = bob

	self.transform.Position = self.startPosition + offset
	self.lastPosition = self.transform.Position

	local axis = self.spinAxis:Normalized()

	if axis:Length() > 0.0 then
		local delta = Quat.FromAxisAngle(axis, math.rad(self.spinSpeed) * dt)
		local current = Quat.FromEuler(self.transform.Rotation)
		self.transform.Rotation = (delta * current):Normalized():ToEuler()
	end

	local scroll = self.uvScroll * dt
	self.scrollAccumulator = (self.scrollAccumulator or Vector2(0, 0)) + scroll

	local forward = self.baseOrientation * Vector3(0, 0, 1)

	if self.mesh ~= nil and self.maxBounces > 0 then
		local shouldHide = math.floor(self.elapsedTime) % 2 == 1
		self.mesh.HideInRender = shouldHide and self.frameCount % 2 == 0 and false or self.mesh.HideInRender
	end

	if self.frameCount % 120 == 0 then
		print(string.format("[%d] t=%.2f height=%.3f forward=%s",
			self.frameCount, self.elapsedTime, self.currentHeight, Describe(forward)))
	end
end

function Showcase:OnStop()
	if self.transform ~= nil then
		self.transform.Position = self.startPosition
		self.transform.Rotation = self.startRotation
	end

	print(string.format("Showcase stopped after %d frames (%.2f s)", self.frameCount, self.elapsedTime))
end

return Showcase
```
</div>
