#include "MaterialGraphView.h"

#include <Editor/Domain/DomainFile.h>
#include <Editor/Renderer/EditorContext.h>
#include <Engine/Asset/Material/MaterialSerializer.h>
#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>
#include <Runtime/Serialization/JsonArchive.h>

#include <imgui.h>

namespace Horizon::Editor
{
	MaterialGraphView::~MaterialGraphView()
	{
	}

	void MaterialGraphView::OnInvoke()
	{
		m_reflection = GetContext()->pEngine->GetReflectionSystem();
		m_propertyRenderer.Initialize(GetContext()->pEngine);
	}

	void MaterialGraphView::OnRender(const Engine::EngineFrame& context)
	{
		DomainFile* pFile = GetConnectedFile();
		if (!pFile)
			return;

		if (pFile != m_loadedFile)
		{
			m_loadedFile = pFile;
			m_isLoaded = LoadSource(pFile);
		}

		if (!m_isLoaded)
		{
			ImGui::TextUnformatted("Material source could not be read");
			return;
		}

		if (ImGui::Button("Save"))
			SaveSource();

		ImGui::SeparatorText("Properties");

		i32 domain = static_cast<i32>(m_properties.domain);
		if (ImGui::Combo("Domain", &domain, "Opaque\0Masked\0Transparent\0"))
			m_properties.domain = static_cast<Asset::MaterialDomain>(domain);

		ImGui::Checkbox("Two Sided", &m_properties.twoSided);

		ImGui::SeparatorText("Parameters");

		const Reflect::Type* pType = m_reflection->GetType(Reflect::TypeOf<Asset::MaterialParameters>());
		if (!pType)
		{
			ImGui::TextUnformatted("MaterialParameters is not a reflected type");
			return;
		}

		m_propertyRenderer.DrawObject(*pType, &m_parameters);
	}

	b8 MaterialGraphView::OnCommand(ViewCommand command)
	{
		if (command != ViewCommand::Save)
			return false;

		if (!m_isLoaded || !m_connectedFile)
			return false;

		return SaveSource();
	}

	b8 MaterialGraphView::LoadSource(DomainFile* pFile)
	{
		m_properties = Asset::MaterialProperties();
		m_parameters = Asset::MaterialParameters();

		const std::string& path = pFile->GetSourcePath();

		PAL::FileAccessRequest request = PAL::File::RequestAccess(path, PAL::FileOperationAccessPolicy::Read,
			PAL::FileOperationSharePolicy::SharedRead);

		if (!request.IsValid())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} cannot be opened", path);
			return false;
		}

		std::string text;
		const b8 wasRead = PAL::File::ReadString(request, text);

		PAL::File::ReleaseAccess(request);

		if (!wasRead)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} cannot be read", path);
			return false;
		}

		JsonArchiveReader reader(text);

		if (!Asset::MaterialSerializer::Deserialize(m_properties, m_parameters, m_reflection, reader) || reader.HasError())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} is not a valid material archive", path);
			return false;
		}

		return true;
	}

	b8 MaterialGraphView::SaveSource()
	{
		JsonArchiveWriter writer;

		if (!Asset::MaterialSerializer::Serialize(m_properties, m_parameters, m_reflection, writer))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Material could not be serialized");
			return false;
		}

		const std::string& path = m_connectedFile->GetSourcePath();

		PAL::FileAccessRequest request = PAL::File::RequestAccess(path, PAL::FileOperationAccessPolicy::Write,
			PAL::FileOperationSharePolicy::Exclusive);

		PAL::File::Truncate(request, 0);

		const b8 wasWritten = PAL::File::WriteString(request, writer.ToString());

		PAL::File::ReleaseAccess(request);

		if (!wasWritten)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} cannot be written", path);
			return false;
		}

		return true;
	}
}