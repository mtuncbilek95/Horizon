#pragma once

namespace Horizon
{
	class Engine;

	class H_EXPORT MenuItem
	{
		friend class MenuRegistry;
	public:
		virtual ~MenuItem() = default;

		virtual b8 IsChecked() const { return false; }
		virtual void OnExecute() = 0;

	protected:
		Engine* m_engine;
	};
}