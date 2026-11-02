#pragma once

#include <Engine/Core/Engine.h>
#include <Engine/Job/JobSystem.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/Rendering/RenderGraph/RenderNode.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/Containers/Memory/RefPtr.h>

namespace Horizon::Engine
{
	class ENGINE_API RenderGraph
	{
	public:
		RenderGraph(Engine* pEngine);
		~RenderGraph();

		b8 OnInitialize();
		void OnExecute();
		void OnFinalize();

		void OnLibraryRegistered(const ReflectionLibrary& library);
		void OnLibraryUnregistered(const ReflectionLibrary& library);

	private:
		Engine* m_engine = nullptr;
		ReflectionSystem* m_reflection = nullptr;
		JobSystem* m_job = nullptr;

		List<RefPtr<RenderNode>> m_activeNodes;
	};
}