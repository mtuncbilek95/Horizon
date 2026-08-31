#include "AssetOpenerRegistry.h"

#include <Editor/Views/AssetBrowserView/AssetOpenerAttribute.h>
#include <Editor/Domain/DomainFile.h>

#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>

#include <utility>

namespace Horizon::Editor
{
	AssetOpenerRegistry::~AssetOpenerRegistry()
	{
		Clear();
	}

	void AssetOpenerRegistry::Bootstrap(Engine::Engine* pEngine)
	{
		Clear();

		auto* pReflect = pEngine->GetReflectionSystem();

		List<Reflect::Type*> types = pReflect->GetTypeByAttribute(Reflect::TypeOf<AssetOpenerAttribute>());

		for (Reflect::Type* pType : types)
		{
			if (pType->GetBaseId() != Reflect::TypeOf<AssetOpener>())
			{
				Terminal::Error("AssetOpenerRegistry", "{} doesn't have AssetOpener as its inheritance.", pType->GetName());
				continue;
			}

			AssetOpenerAttribute* pAttribute = pType->GetCustomAttribute<AssetOpenerAttribute>();

			if (pAttribute == nullptr)
			{
				Terminal::Error("AssetOpenerRegistry", "{} carries no asset opener attribute", pType->GetName());
				continue;
			}

			Reflect::Type* pAssetType = pReflect->GetType(pAttribute->GetAssetHandle());

			if (pAssetType == nullptr)
			{
				Terminal::Error("AssetOpenerRegistry", "{} targets an unregistered asset type", pType->GetName());
				continue;
			}

			OpenerEntry entry;
			entry.pOpener = (AssetOpener*)pType->Create();
			entry.assetTypeName = pAssetType->GetName();

			Terminal::Info("AssetOpenerRegistry", "{} opens {}", pType->GetName(), entry.assetTypeName);

			m_openers.PushBack(std::move(entry));
		}
	}

	void AssetOpenerRegistry::Clear()
	{
		for (OpenerEntry& entry : m_openers)
			Memory::Allocator::Delete(entry.pOpener);

		m_openers.Clear();
	}

	b8 AssetOpenerRegistry::CanOpen(std::string_view assetTypeName) const
	{
		return Find(assetTypeName) != nullptr;
	}

	b8 AssetOpenerRegistry::Open(Engine::Engine* pEngine, DomainFile* pFile) const
	{
		if (pFile == nullptr)
			return false;

		const std::string& assetTypeName = pFile->GetMeta().assetTypeName;
		AssetOpener* pOpener = Find(assetTypeName);

		if (pOpener == nullptr)
		{
			Terminal::Info("AssetOpenerRegistry", "{} is a {} and cannot be opened yet", pFile->GetName(), assetTypeName);
			return false;
		}

		return pOpener->Open(pEngine, pFile);
	}

	AssetOpener* AssetOpenerRegistry::Find(std::string_view assetTypeName) const
	{
		for (const OpenerEntry& entry : m_openers)
		{
			if (StringOps::EqualsNoCase(entry.assetTypeName, assetTypeName))
				return entry.pOpener;
		}

		return nullptr;
	}
}