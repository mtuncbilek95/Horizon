#pragma once

namespace Horizon
{
	class IModule
	{
	public:
		virtual ~IModule() = default;

		virtual b8 Initialize() = 0;
		virtual void Update() = 0;
		virtual void Finalize() = 0;
	};
}