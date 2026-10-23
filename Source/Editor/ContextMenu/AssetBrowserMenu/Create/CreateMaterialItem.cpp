#include "CreateMaterialItem.h"

#include <Engine/Reflection/ReflectionSystem.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>
#include <Runtime/Serialization/JsonArchive.h>

#include <format>

namespace Horizon::Editor
{
	namespace
	{
		b8 HasFileNamed(const DomainFolder* pParent, const std::string& name)
		{
			for (const DomainFile* pFile : pParent->GetFiles())
			{
				if (StringOps::EqualsNoCase(pFile->GetName(), name))
					return true;
			}

			return PAL::File::Exists(pParent->GetAbsolutePath() + "/" + name);
		}

		std::string ResolveUniqueName(const DomainFolder* pParent)
		{
			std::string fileName = "NewMaterial.hmaterial";

			if (!HasFileNamed(pParent, fileName))
				return fileName;

			i32 index = 1;

			do
			{
				fileName = std::format("NewMaterial({}).hmaterial", index);
				index++;
			} while (HasFileNamed(pParent, fileName));

			return fileName;
		}
	}

	void CreateMaterialItem::OnExecute(AssetBrowserContext& context)
	{
		Engine::ReflectionSystem* pReflection = context.pEngine->GetReflectionSystem();

		const std::string path = context.pCurrentFolder->GetAbsolutePath() + "/" + ResolveUniqueName(context.pCurrentFolder);
		if (!PAL::File::Create(path))
		{
			Terminal::Error(StringOps::GetName(this), "{} cannot be created", path);
			return;
		}

		/*PAL::FileAccessRequest request = PAL::File::RequestAccess(path, PAL::FileOperationAccessPolicy::Write,
			PAL::FileOperationSharePolicy::Exclusive);

		const b8 wasWritten = PAL::File::WriteString(request, writer.ToString());

		PAL::File::ReleaseAccess(request);

		if (!wasWritten)
			Terminal::Error(StringOps::GetName(this), "{} cannot be written", path);*/
	}

	b8 CreateMaterialItem::IsEnabled(const AssetBrowserContext& context)
	{
		return context.selectedFiles.GetCount() == 0 && context.selectedFolders.GetCount() == 0;
	}
}