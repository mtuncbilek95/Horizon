#pragma once

namespace Horizon
{
	template<typename T>
	class Singleton
	{
		friend class ModuleSystem;
	public:
		static T& Get()
		{
			static T instance;
			return instance;
		}

		static void SetInstance
	};
}