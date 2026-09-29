#pragma once

#include <Engine/Core/Service.h>

namespace JPH
{
	class PhysicsSystem;
	class TempAllocator;
	class JobSystem;
}

namespace Horizon::Engine
{
	class H_EXPORT PhysicsService final : public Service
	{
		static constexpr u32 MaxBodies = 65536u;
		static constexpr u32 MaxBodyPairs = 65536u;
		static constexpr u32 MaxContactConstraints = 10240u;
		static constexpr u32 MaxPreAllocatedMemory = MibToByte(16u);
		static constexpr u32 MaxPhysicsBarrierCount = 8;

	public:
		static constexpr f32 FixedStep = 1.0f / 60.0f;
		static constexpr u32 MaxStepsPerFrame = 4;

		ModuleReport OnInitialize() final;
		void OnExecute(const EngineFrame& ctx) final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleGraph& graph) final;

		JPH::PhysicsSystem* GetWorld() const;
		JPH::TempAllocator* GetTempAllocator() const;
		JPH::JobSystem* GetJobBridge() const;

	private:
		struct Impl;
		Impl* m_impl = nullptr;
	};
}