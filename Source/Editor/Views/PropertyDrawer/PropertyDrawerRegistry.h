#pragma once

#include <Editor/Views/PropertyDrawer/PropertyDrawer.h>
#include <Engine/Core/Engine.h>

#include <unordered_map>

namespace Horizon::Editor
{
	class H_EXPORT PropertyDrawerRegistry
	{
	public:
		PropertyDrawerRegistry() = default;
		~PropertyDrawerRegistry();

		void BootstrapDrawers(Engine::Engine* pEngine);
		PropertyDrawer* Find(Reflect::TypeHandle typeId) const;

	private:
		void Clear();

	private:
		std::unordered_map<Reflect::TypeHandle, PropertyDrawer*> m_drawers;
	};
}