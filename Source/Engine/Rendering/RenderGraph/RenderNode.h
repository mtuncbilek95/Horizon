#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class Engine;
	class RenderDependency;

	HCLASS();
	class ENGINE_API RenderNode : public Reflect::Base
	{
		friend class RenderGraph;
	public:
		virtual b8 OnInitialize() = 0;
		virtual void OnFinalize() = 0;
		virtual void Setup(RenderDependency& graph) = 0;
		virtual void Render() = 0;

		Engine* GetEngine() const { return m_engine; }

	private:
		Engine* m_engine = nullptr;
	};
}