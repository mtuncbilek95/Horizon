#pragma once

#include <Engine/Core/Service.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/Math/Vec2f.h>

#include <string>
#include <string_view>
#include <unordered_map>

#include <Runtime/Containers/Memory/UniquePtr.h>
#include <Runtime/Input/Instance.h>

namespace Horizon::Engine
{
	class ENGINE_API InputService final : public Service
	{
	public:
		ModuleReport OnInitialize() final;
		void OnExecute(const EngineFrame& ctx) final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleDependency& graph) final;

		void OnLibraryRegistered(const ReflectionLibrary& library) final;
		void OnLibraryUnregistered(const ReflectionLibrary& library) final;

		Input::Instance* GetDevices() const { return m_devices.Get(); }

	private:
		UniquePtr<Input::Instance> m_devices;
	};
}