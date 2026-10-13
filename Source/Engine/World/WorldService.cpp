#include "WorldService.h"

#include <Engine/Asset/AssetService.h>
#include <Engine/Asset/Scene/SceneSerializer.h>
#include <Engine/Core/Engine.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/System.h>
#include <Engine/World/SystemOrderAttribute.h>
#include <Engine/World/ECS/Scene.h>
#include <Runtime/Serialization/BinaryArchive.h>

namespace Horizon::Engine
{
	ModuleReport WorldService::OnInitialize()
	{
		m_reflection = GetEngine()->GetReflectionSystem();

		for (const Reflect::Type* pType : m_reflection->GetTypeByBase(Reflect::TypeOf<System>()))
			AddSystemType(pType);

		SortSystems();
		return ModuleReport();
	}

	void WorldService::OnExecute(const EngineFrame& ctx)
	{
		if (m_snapshotWorld)
			RestoreSnapshot();

		if (!m_activeWorld)
			return;

		for (SystemEntry& entry : m_systems)
		{
			if (HasFlag(entry.pSystem->GetWorkingGroup(), m_runningSystems))
				entry.pSystem->OnExecute(ctx, *m_activeWorld);
		}
	}

	void WorldService::OnFinalize()
	{
		for (SystemEntry& entry : m_systems)
		{
			entry.pSystem->OnFinalize();
			entry.pType->Destroy(entry.pSystem);
		}

		m_systems.Clear();
	}

	void WorldService::DeclareDependencies(ModuleGraph& graph)
	{
		graph.Requires<AssetService>();
		graph.Requires<GraphicsContext>();
	}

	void WorldService::OnLibraryRegistered(const ReflectionLibrary& library)
	{
		for (const Reflect::Type& type : library.types)
		{
			if (type.GetBaseId() == Reflect::TypeOf<System>())
				AddSystemType(&type);
		}

		SortSystems();
	}

	void WorldService::OnLibraryUnregistered(const ReflectionLibrary& library)
	{
		CaptureSnapshot();

		for (const Reflect::Type& type : library.types)
			RemoveSystemType(&type);

		if (m_snapshotWorld)
			m_snapshotWorld->Clear();
	}

	System* WorldService::RequestSystem(Reflect::TypeHandle handl) const
	{
		auto it = m_systemLookup.find(handl);

		if (it == m_systemLookup.end())
			return nullptr;

		return it->second;
	}

	b8 WorldService::AddSystemType(const Reflect::Type* pType)
	{
		auto* pAttr = pType->GetCustomAttribute<SystemOrderAttribute>();

		if (!pAttr)
		{
			Terminal::Error(StringOps::GetName(this), "{} has no SystemOrderAttribute. You won't have this system.", pType->GetName());
			return false;
		}

		auto* pSystem = (System*)pType->Create();

		if (!pSystem)
		{
			Terminal::Error(StringOps::GetName(this), "{} has virtual function issues. You won't have this system.", pType->GetName());
			return false;
		}

		pSystem->m_engine = GetEngine();
		pSystem->m_ownerService = this;

		if (!pSystem->OnInitialize())
		{
			Terminal::Error(StringOps::GetName(this), "{} failed to initialize. You won't have this system.", pType->GetName());
			pType->Destroy(pSystem);
			return false;
		}

		m_systemLookup[pSystem->GetTypeId()] = pSystem;
		m_systems.EmplaceBack(pSystem, pType, pAttr->GetOrderNumber());

		Terminal::Info(StringOps::GetName(this), "{} has been registered to WorldService.", pType->GetName());
		return true;
	}

	void WorldService::RemoveSystemType(const Reflect::Type* pType)
	{
		for (usize i = m_systems.GetCount(); i > 0; --i)
		{
			SystemEntry& entry = m_systems[i - 1];

			if (entry.pType != pType)
				continue;

			m_systemLookup.erase(entry.pSystem->GetTypeId());
			entry.pSystem->OnFinalize();
			entry.pType->Destroy(entry.pSystem);
			m_systems.RemoveAt(i - 1);
		}
	}

	void WorldService::SortSystems()
	{
		m_systems.Sort([](const SystemEntry& a, const SystemEntry& b)
			{
				return a.order < b.order;
			});
	}

	void WorldService::SetRunningSystems(SystemGroup groups)
	{
		if (groups == m_runningSystems)
			return;

		const SystemGroup previous = m_runningSystems;
		m_runningSystems = groups;

		for (SystemEntry& entry : m_systems)
			entry.pSystem->OnGroupsChanged(previous, groups);
	}

	void WorldService::CaptureSnapshot()
	{
		if (!m_activeWorld || m_snapshotWorld)
			return;

		BinaryArchiveWriter writer;
		SceneSerializer::Serialize(*m_activeWorld, m_reflection, writer);

		if (!writer.IsComplete())
		{
			Terminal::Error(StringOps::GetName(this), "Active world could not be captured, its content will be lost");
			m_snapshotWorld = m_activeWorld;
			return;
		}

		m_snapshot = writer.GetBytes();
		m_snapshotWorld = m_activeWorld;
	}

	void WorldService::RestoreSnapshot()
	{
		Scene* pTarget = m_snapshotWorld;
		m_snapshotWorld = nullptr;

		if (m_snapshot.IsEmpty())
			return;

		if (pTarget != m_activeWorld)
		{
			Terminal::Warn(StringOps::GetName(this), "Active world changed before restore, snapshot is dropped");
			m_snapshot.Clear();
			return;
		}

		BinaryArchiveReader reader(m_snapshot.GetData(), m_snapshot.GetCount());

		if (!SceneSerializer::Deserialize(*pTarget, m_reflection, reader))
			Terminal::Error(StringOps::GetName(this), "Active world could not be restored completely");

		m_snapshot.Clear();
	}
}