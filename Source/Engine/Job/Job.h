#pragma once

#include <functional>
#include <atomic>

namespace Horizon
{
	using Job = std::function<void()>;
}
