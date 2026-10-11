#include "ScriptAssetImporter.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>

#include <algorithm>
#include <cstring>

namespace Horizon::Editor
{
	static constexpr u8 Utf8Bom[] = { 0xEF, 0xBB, 0xBF };

	static b8 HasUtf8Bom(const List<u8>& bytes)
	{
		if (bytes.GetCount() < sizeof(Utf8Bom))
			return false;

		return std::memcmp(bytes.GetData(), Utf8Bom, sizeof(Utf8Bom)) == 0;
	}

	static std::string FileNameOf(const std::string& path)
	{
		const usize separator = path.find_last_of("/\\");

		if (separator == std::string::npos)
			return path;

		return path.substr(separator + 1);
	}

	AssetImportResult ScriptAssetImporter::ImportAsset(Engine::Engine* pEngine, const std::string& inPath, List<u8>& outByteArr)
	{
		PAL::FileAccessRequest request = PAL::File::RequestAccess(inPath, PAL::FileOperationAccessPolicy::Read,
			PAL::FileOperationSharePolicy::SharedRead);

		if (!request.IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "{} cannot be opened", inPath);
			return AssetImportResult::FileNotFound;
		}

		List<u8> source;
		const b8 wasRead = PAL::File::ReadMemory(request, source);

		PAL::File::ReleaseAccess(request);

		if (!wasRead)
		{
			Terminal::Error(StringOps::GetName(this), "{} cannot be read", inPath);
			return AssetImportResult::FileReadFailed;
		}

		const usize sourceOffset = HasUtf8Bom(source) ? sizeof(Utf8Bom) : 0;
		const usize sourceSize = source.GetCount() - sourceOffset;

		if (sourceSize == 0)
		{
			Terminal::Error(StringOps::GetName(this), "{} is empty", inPath);
			return AssetImportResult::EmptyInput;
		}

		Engine::ScriptProperties properties = {};
		properties.sourceSize = sourceSize;

		const std::string fileName = FileNameOf(inPath);
		std::memcpy(properties.chunkName, fileName.data(), std::min(fileName.size(), Engine::ScriptProperties::MaxChunkName - 1));

		const usize propertyBytes = sizeof(Engine::ScriptProperties);

		outByteArr.Resize(propertyBytes + sourceSize);

		u8* pCursor = outByteArr.GetData();

		std::memcpy(pCursor, &properties, propertyBytes);
		pCursor += propertyBytes;

		std::memcpy(pCursor, source.GetData() + sourceOffset, sourceSize);

		return AssetImportResult::Success;
	}
}