#pragma once

#include <Editor/Renderer/EditorContext.h>
#include <Engine/Core/EngineFrame.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	class EDITOR_API ProjectSection : public Reflect::Base
	{
	public:
		virtual ~ProjectSection() = default;

		void Initialize(EditorContext* pContext)
		{
			m_context = pContext;
			OnInitialize();
		}

		virtual void OnInitialize() = 0;
		virtual void OnRender(const Engine::EngineFrame& context) = 0;
		virtual void OnFinalize() = 0;

		EditorContext* GetContext() const { return m_context; }

	private:
		EditorContext* m_context = nullptr;
	};
}