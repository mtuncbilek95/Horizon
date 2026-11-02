#include "InputEvaluation.h"

namespace Horizon::World::System
{
	b8 InputEvaluation::OnInitialize()
	{
		return true;
	}

	void InputEvaluation::OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene)
	{
	}

	void InputEvaluation::OnFinalize()
	{
	}

	void InputEvaluation::DeclareDependencies(Engine::SystemDependency& graph)
	{
	}
}