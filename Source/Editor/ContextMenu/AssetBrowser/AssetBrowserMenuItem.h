#pragma once

#include <Editor/Core/IEditorItem.h>
#include <Editor/Widget/AssetBrowser/AssetBrowserContext.h>

namespace Horizon
{
	class AssetBrowserMenuItem : public IEditorItem
	{
	public:
		virtual ~AssetBrowserMenuItem() = default;

		virtual void OnInvoke(const AssetBrowserContext& context) = 0;
	};
}