#pragma once

#include <Editor/Renderer/EditorContext.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	class EDITOR_API MenuItem : public Reflect::Base
	{
		friend class MenuRegistry;
	public:
		virtual ~MenuItem() = default;

		virtual void OnExecute() = 0;

		EditorContext* GetContext() const noexcept { return m_context; }

	private:
		EditorContext* m_context = nullptr;
	};
}