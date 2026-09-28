#pragma once

#include <Editor/Renderer/EditorContext.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	class H_EXPORT ToolBarItem : public Reflect::Base
	{
		friend class ToolBarRegistry;
	public:
		virtual ~ToolBarItem() = default;

		virtual void OnRender() = 0;

		EditorContext* GetContext() const { return m_context; }

	private:
		EditorContext* m_context = nullptr;
	};
}