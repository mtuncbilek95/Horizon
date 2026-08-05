#include "ViewRegistry.h"

#include <Editor/Attributes/EditorViewAttribute.h>

#include <Engine/Core/Engine.h>
#include <Engine/Module/ModuleContext.h>

#include <imgui.h>

namespace Horizon
{
	ViewRegistry::ViewRegistry(Engine* pEngine) : m_engine(pEngine)
	{
	}

	ViewRegistry::~ViewRegistry()
	{
		for (auto* view : m_createdViews)
			Allocator::Delete(view);

		m_createdViews.Clear();
		m_registeredViews.Clear();
	}

	void ViewRegistry::BootstrapViews()
	{
		if (!m_engine)
		{
			Terminal::Fatal("ViewRegistry", "Somehow engine is not there!");
			return;
		}

		for (auto* view : m_createdViews)
			Allocator::Delete(view);

		m_createdViews.Clear();
		m_registeredViews.Clear();

		auto* moduleCtx = m_engine->GetModuleContext();

		List<Reflect::Type*> types = moduleCtx->GetTypeByAttribute(Reflect::TypeOf<EditorViewAttribute>());

		for (auto* type : types)
		{
			auto* pAttr = type->GetCustomAttribute<EditorViewAttribute>();

			m_registeredViews.PushBack(ViewDescriptor
				{
					.displayName = pAttr->GetDisplayName(),
					.multiInstance = pAttr->GetMultiInstance(),
					.openOnStart = pAttr->GetOpenOnStart(),
					.pCoreType = type
				});
		}

		for (const auto& view : m_registeredViews)
		{
			if (view.openOnStart)
			{
				auto* viewObj = (ViewObject*)view.pCoreType->CreateFromMemory();
				viewObj->m_engine = m_engine;
				viewObj->m_displayName = view.displayName;

				m_createdViews.PushBack(viewObj);
			}
		}
	}

	void ViewRegistry::RenderGUI()
	{
		for (auto* view : m_createdViews)
		{
			ImGui::Begin(view->m_displayName.c_str());
			view->OnRender();
			ImGui::End();
		}
	}
}