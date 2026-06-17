#pragma once

namespace Horizon
{
	class ReflectionModule : public IModule
	{
	public:
		void OnAttach(Engine& engine);
		void OnDetach();

	private:

	};
}