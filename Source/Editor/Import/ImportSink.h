#pragma once

#include <Runtime/Containers/Guid.h>
#include <string>
#include <string_view>

namespace Horizon::Engine
{
	class ContentFileWriter;
}

namespace Horizon::Editor
{
	class H_EXPORT ImportSink
	{
	public:
		virtual ~ImportSink() = default;

		virtual Guid ResolveSubAsset(std::string_view name, std::string_view assetTypeName) = 0;
		virtual Engine::ContentFileWriter* Open(const Guid& guid, std::string_view assetTypeName) = 0;
		virtual void Close(Engine::ContentFileWriter* pWriter) = 0;
	};

}