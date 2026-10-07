#include "PhysicsService.h"

#include <Engine/Core/Engine.h>
#include <Engine/Job/JobSystem.h>
#include <Engine/Physics/JoltThreadBridge.h>
#include <Engine/World/WorldService.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

#include <cstdarg>
#include <cstdio>

namespace Horizon::Engine
{
	namespace Layers
	{
		static constexpr JPH::ObjectLayer Static = 0;
		static constexpr JPH::ObjectLayer Moving = 1;
		static constexpr JPH::ObjectLayer Count = 2;
	}

	namespace BroadPhaseLayers
	{
		static constexpr JPH::BroadPhaseLayer Static(0);
		static constexpr JPH::BroadPhaseLayer Moving(1);
		static constexpr JPH::uint Count = 2;
	}

	class BroadPhaseLayerMap final : public JPH::BroadPhaseLayerInterface
	{
	public:
		JPH::uint GetNumBroadPhaseLayers() const final { return BroadPhaseLayers::Count; }
		JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const final { return layer == Layers::Static ? BroadPhaseLayers::Static : BroadPhaseLayers::Moving; }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const final { return layer == BroadPhaseLayers::Static ? "Static" : "Moving"; }
#endif
	};

	class ObjectVsBroadPhaseFilter final : public JPH::ObjectVsBroadPhaseLayerFilter
	{
	public:
		bool ShouldCollide(JPH::ObjectLayer layer, JPH::BroadPhaseLayer bpLayer) const final { return layer != Layers::Static || bpLayer != BroadPhaseLayers::Static; }
	};

	class ObjectPairFilter final : public JPH::ObjectLayerPairFilter
	{
	public:
		bool ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const final { return a != Layers::Static || b != Layers::Static; }
	};

	struct PhysicsService::Impl
	{
		BroadPhaseLayerMap broadPhaseMap;
		ObjectVsBroadPhaseFilter objectVsBroadPhase;
		ObjectPairFilter objectPair;

		JoltThreadBridge* pBridge = nullptr;
		JPH::TempAllocatorImpl* pTemp = nullptr;
		JPH::PhysicsSystem world;
	};

	static void BindJoltHooks()
	{
		JPH::Allocate = [](size_t size) -> void*
			{
				return Memory::Allocator::AllocateRaw(size, 16, Memory::CurrLoc());
			};

		JPH::Free = [](void* pBlock)
			{
				Memory::Allocator::FreeRaw(pBlock);
			};

		JPH::AlignedAllocate = [](size_t size, size_t alignment) -> void*
			{
				return Memory::Allocator::AllocateRaw(size, alignment, Memory::CurrLoc());
			};

		JPH::AlignedFree = [](void* pBlock)
			{
				Memory::Allocator::FreeRaw(pBlock);
			};

		JPH::Reallocate = [](void* pBlock, size_t oldSize, size_t newSize) -> void*
			{
				if (!pBlock)
					return Memory::Allocator::AllocateRaw(newSize, 16, Memory::CurrLoc());

				return Memory::Allocator::ReallocateRaw(pBlock, newSize, 16, Memory::CurrLoc());
			};

		JPH::Trace = [](const char* pFormat, ...)
			{
				char buffer[1024];

				va_list args;
				va_start(args, pFormat);
				vsnprintf(buffer, sizeof(buffer), pFormat, args);
				va_end(args);

				Terminal::Info("Jolt", "{}", buffer);
			};

#ifdef JPH_ENABLE_ASSERTS
		JPH::AssertFailed = [](const char* pExpression, const char* pMessage, const char* pFile, JPH::uint line) -> bool
			{
				Terminal::Error("Jolt", "{} ({}:{}) {}", pExpression, pFile, line, pMessage ? pMessage : "");
				return true;
			};
#endif
	}


	ModuleReport PhysicsService::OnInitialize()
	{
		BindJoltHooks();

		JPH::Factory::sInstance = Memory::Allocator::Create<JPH::Factory>(Memory::CurrLoc());
		JPH::RegisterTypes();

		m_impl = Memory::Allocator::Create<Impl>(Memory::CurrLoc());
		if (!m_impl)
			return ModuleReport("Failed to allocate physics state");

		m_impl->pTemp = Memory::Allocator::Create<JPH::TempAllocatorImpl>(Memory::CurrLoc(), MaxPreAllocatedMemory);
		m_impl->pBridge = Memory::Allocator::Create<JoltThreadBridge>(Memory::CurrLoc(), GetEngine()->GetJobSystem(), MaxPhysicsBarrierCount);

		m_impl->world.Init(MaxBodies, 0, MaxBodyPairs, MaxContactConstraints, m_impl->broadPhaseMap, m_impl->objectVsBroadPhase, m_impl->objectPair);

		Terminal::Debug(StringOps::GetName(this), "Jolt world initialized with {} worker lanes", m_impl->pBridge->GetMaxConcurrency());
		return ModuleReport();
	}

	void PhysicsService::OnExecute(const EngineFrame& ctx)
	{
	}

	void PhysicsService::OnFinalize()
	{
		if (m_impl)
		{
			Memory::Allocator::Delete(m_impl->pBridge);
			Memory::Allocator::Delete(m_impl->pTemp);
			Memory::Allocator::Delete(m_impl);
			m_impl = nullptr;
		}

		JPH::UnregisterTypes();

		Memory::Allocator::Delete(JPH::Factory::sInstance);
		JPH::Factory::sInstance = nullptr;
	}

	void PhysicsService::DeclareDependencies(ModuleGraph& graph)
	{
		graph.Precedes<WorldService>();
	}

	JPH::PhysicsSystem* PhysicsService::GetWorld() const
	{
		return m_impl ? &m_impl->world : nullptr;
	}

	JPH::TempAllocator* PhysicsService::GetTempAllocator() const
	{
		return m_impl ? m_impl->pTemp : nullptr;
	}

	JPH::JobSystem* PhysicsService::GetJobBridge() const
	{
		return m_impl ? m_impl->pBridge : nullptr;
	}
}