#pragma once

#include <Editor/Views/ViewCommand.h>
#include <Engine/Core/EngineFrame.h>
#include <Runtime/RTTR/Reflection.h>
#include <string>

namespace Horizon::Engine
{
	class Engine;
}

namespace Horizon::Editor
{
	class ViewRegistry;
	class EditorContext;
	class DomainFile;

	class H_EXPORT ViewObject : public Reflect::Base
	{
		friend class ViewRegistry;
	public:
		virtual ~ViewObject() = default;

		virtual void OnInvoke() = 0;
		virtual void OnRender(const Engine::EngineFrame& context) = 0;
		virtual b8 OnCommand(ViewCommand command) { return false; }

		virtual b8 IsFullBleed() const { return false; }

		ViewRegistry* GetRegistry() const { return m_holder; }
		EditorContext* GetContext() const { return m_context; }

		const std::string& GetDisplayName() const { return m_displayName; }

		DomainFile* GetConnectedFile() const { return m_connectedFile; }
		void SetConnectedFile(DomainFile* pFile) { m_connectedFile = pFile; }

	protected:
		ViewRegistry* m_holder = nullptr;
		EditorContext* m_context = nullptr;
		DomainFile* m_connectedFile = nullptr;

		std::string m_displayName;
	};
}