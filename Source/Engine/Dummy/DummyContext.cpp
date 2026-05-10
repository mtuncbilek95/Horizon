#include "DummyContext.h"

namespace Horizon
{
	EngineReport DummyContext::OnInitialize()
	{

		return EngineReport();
	}

	void DummyContext::OnFinalize()
	{}
}