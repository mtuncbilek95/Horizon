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

`PS5 and XBOX support has been marked as W.I.P.`

## BASIC INFORMATION
Horizon has it's own abstraction structures depending on the concept. Such as:
- RHI
- PAL
- IService (named as Service, inherits from Module)
- IContext (named as Context, inherits from Module)

Almost every abstraction has a clean custom implementation without depending on a third party library to make multiple structure user-friendly. Which means that PAL has a native
Win32API implementation. RHI is not nvrhi. The reason for that is ----> "Why the hell not?"
</div>
