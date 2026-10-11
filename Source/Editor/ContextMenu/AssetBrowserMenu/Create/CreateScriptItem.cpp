#include "CreateScriptItem.h"

#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/ECS/Scene.h>
#include <Engine/Asset/Scene/SceneSerializer.h>
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
			std::string fileName = "NewScript.hscript";

			if (!HasFileNamed(pParent, fileName))
				return fileName;

			i32 index = 1;

			do
			{
				fileName = std::format("NewScript({}).hscript", index);
				index++;
			} while (HasFileNamed(pParent, fileName));

			return fileName;
		}

		std::string GenerateLuaScript(const std::string& scriptName)
		{
			return std::format(
				"local {0} = {{}}\n"
				"\n"
				"{0}.Properties = {{\n"
				"}}\n"
				"\n"
				"function {0}:OnStart()\n"
				"end\n"
				"\n"
				"function {0}:OnUpdate(dt)\n"
				"end\n"
				"\n"
				"function {0}:OnStop()\n"
				"end\n"
				"\n"
				"return {0}\n",
				scriptName);
		}
	}

	void CreateScriptItem::OnExecute(AssetBrowserContext& context)
	{
		Engine::ReflectionSystem* pReflection = context.pEngine->GetReflectionSystem();

		std::string resolveName = ResolveUniqueName(context.pCurrentFolder);
		const std::string path = context.pCurrentFolder->GetAbsolutePath() + "/" + resolveName;

		if (!PAL::File::Create(path))
		{
			Terminal::Error(StringOps::GetName(this), "{} cannot be created", path);
			return;
		}

		std::string luaCode = GenerateLuaScript(resolveName.substr(0,resolveName.find(".")));

		PAL::FileAccessRequest request = PAL::File::RequestAccess(path, PAL::FileOperationAccessPolicy::Write,
			PAL::FileOperationSharePolicy::Exclusive);

		const b8 wasWritten = PAL::File::WriteString(request, luaCode);

		PAL::File::ReleaseAccess(request);

		if (!wasWritten)
			Terminal::Error(StringOps::GetName(this), "{} cannot be written", path);
	}

	b8 CreateScriptItem::IsEnabled(const AssetBrowserContext& context)
	{
		return context.selectedFiles.GetCount() == 0 && context.selectedFolders.GetCount() == 0;
	}
}