#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon
{
	class DomainObject
	{
	public:
		~DomainObject() = default;

		b8 IsMarked() const { return m_marked; }

		void Mark() { m_marked = true; }
		void Unmark() { m_marked = false; }

	private:
		b8 m_marked = false;
	};
}