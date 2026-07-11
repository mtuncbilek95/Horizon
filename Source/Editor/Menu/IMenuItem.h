#pragma once

#include <Editor/Core/IEditorItem.h>

namespace Horizon
{
	class IMenuItem : public IEditorItem
	{
	public:
		virtual ~IMenuItem() = default;

		virtual void OnInvoke() = 0;
	};
}