#pragma once

#include <Editor/Components/PropertyDrawer.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>
#include <unordered_map>

namespace Horizon::Engine
{
	class Engine;
}

namespace Horizon::Editor
{
	class EDITOR_API PropertyRenderer final
	{
	public:
		PropertyRenderer() = default;
		~PropertyRenderer();

		PropertyRenderer(const PropertyRenderer&) = delete;
		PropertyRenderer& operator=(const PropertyRenderer&) = delete;

		void Initialize(Engine::Engine* pEngine);

		void DrawObject(const Reflect::Type& type, void* pInstance);

	private:
		void DrawFields(const Reflect::Type& type, void* pInstance);
		void DrawField(const Reflect::Field& field, void* pValue);
		void DrawNested(const Reflect::Field& field, void* pValue);
		void DrawHeaderRow(const std::string& header);

		PropertyDrawer* FindDrawer(Reflect::TypeHandle handle) const;
		static std::string ToDisplayLabel(const std::string& fieldName);

	private:
		PropertyContext m_context;

		List<PropertyDrawer*> m_drawers;
		std::unordered_map<Reflect::TypeHandle, usize> m_drawerLookups;
	};
}