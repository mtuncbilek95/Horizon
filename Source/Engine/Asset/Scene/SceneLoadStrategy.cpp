#include "SceneLoadStrategy.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Serialization/BinaryArchive.h>

#include <string>

namespace Horizon::Engine
{
	Asset* SceneLoadStrategy::Create(List<u8>&& payload)
	{
		auto* pScene = Memory::Allocator::Create<SceneAsset>(Memory::CurrLoc());

		if (!pScene)
		{
			Terminal::Error(StringOps::GetName(this), "Scene asset could not be allocated");
			return nullptr;
		}

		pScene->m_blob = std::move(payload);

		if (!Parse(*pScene))
		{
			Memory::Allocator::Delete(pScene);
			return nullptr;
		}

		return pScene;
	}

	b8 SceneLoadStrategy::Parse(SceneAsset& scene)
	{
		BinaryArchiveReader reader(scene.m_blob.GetData(), scene.m_blob.GetCount());

		if (reader.HasError())
			return false;

		reader.BeginObject();

		if (!reader.Key("version"))
		{
			Terminal::Error(StringOps::GetName(this), "Scene carries no version");
			return false;
		}

		const u64 version = reader.ReadU64();

		if (version > SceneFormatVersion)
		{
			Terminal::Error(StringOps::GetName(this), "Scene version {} is newer than the supported {}", version, SceneFormatVersion);
			return false;
		}

		if (reader.Key("typeNames"))
		{
			const usize count = reader.BeginArray();
			scene.m_typeNames.Resize(count);

			for (usize i = 0; i < count; i++)
				scene.m_typeNames[i] = reader.ReadString();

			reader.EndArray();
		}

		if (reader.Key("dependencies"))
		{
			const usize count = reader.BeginArray();
			scene.m_dependencies.Resize(count);

			for (usize i = 0; i < count; i++)
				scene.m_dependencies[i] = Guid(reader.ReadString());

			reader.EndArray();
		}

		if (reader.Key("entityCount"))
			scene.m_entityCount = (u32)reader.ReadU64();

		if (reader.Key("chunks"))
		{
			const usize count = reader.BeginArray();
			scene.m_chunks.Resize(count);

			for (usize i = 0; i < count; i++)
			{
				SceneChunkDesc& chunk = scene.m_chunks[i];

				reader.BeginObject();

				if (reader.Key("typeIndex"))
					chunk.typeIndex = (u32)reader.ReadU64();

				if (reader.Key("count"))
					chunk.count = (u32)reader.ReadU64();

				if (reader.Key("stride"))
					chunk.stride = (u32)reader.ReadU64();

				if (reader.Key("flags"))
					chunk.flags = (u32)reader.ReadU64();

				if (reader.Key("entities"))
				{
					usize size = 0;
					chunk.entityOffset = reader.ViewBytes(size);

					if (size != chunk.count * sizeof(u32))
					{
						Terminal::Error(StringOps::GetName(this), "Chunk {} carries {} entity bytes for {} elements", i, size, chunk.count);
						return false;
					}
				}

				if (chunk.flags & SceneChunkFlagBlittable)
				{
					if (reader.Key("data"))
					{
						usize size = 0;
						chunk.dataOffset = reader.ViewBytes(size);
						chunk.dataSize = size;
					}
				}
				else if (reader.Key("objects"))
				{
					chunk.dataOffset = reader.GetCursor();
					chunk.dataSize = 0;
				}

				reader.EndObject();
			}

			reader.EndArray();
		}

		reader.EndObject();

		if (reader.HasError())
		{
			Terminal::Error(StringOps::GetName(this), "Scene archive is malformed");
			return false;
		}

		Terminal::Info(StringOps::GetName(this), "Scene parsed with {} entities across {} chunks", scene.m_entityCount, scene.m_chunks.GetCount());
		return true;
	}

	void SceneLoadStrategy::Destroy(Asset* pAsset)
	{
		Memory::Allocator::Delete(pAsset);
	}

	b8 SceneLoadStrategy::Activate(Asset* pAsset)
	{
		return pAsset != nullptr;
	}

	void SceneLoadStrategy::Deactivate(Asset* pAsset)
	{
	}

	void SceneLoadStrategy::OnRegister(Engine* pEngine)
	{
	}

	void SceneLoadStrategy::OnUnregister(Engine* pEngine)
	{
	}
}