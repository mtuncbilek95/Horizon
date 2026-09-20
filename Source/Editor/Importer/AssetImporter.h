#pragma once

#include <Editor/Importer/AssetImportResult.h>
#include <Engine/Core/Engine.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>
#include <string>

namespace Horizon::Editor
{
	class H_EXPORT AssetImporter : public Reflect::Base
	{
	public:
		virtual AssetImportResult ImportAsset(Engine::Engine* pEngine, const std::string& inPath, List<u8>& outByteArr) = 0;
		virtual usize GetPropertySize() const { return 0; }
	};
}