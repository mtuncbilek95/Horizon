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
#### NOTE 3: Vulkan support has been added by [Yunus a.k.a Dnm](https://github.com/Ynsefkksl/). Huge kudos to him.

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
