#include "AssetHandlePropertyDrawer.h"

#include <Editor/Domain/DomainService.h>

#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/RTTR/Attributes/AssetRefAttribute.h>

#include <imgui.h>

namespace Horizon::Editor
{
	b8 AssetHandlePropertyDrawer::OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx)
	{
		Guid* pId = static_cast<Guid*>(pValue);
		const auto* pRef = field.GetCustomAttribute<Reflect::AssetRefAttribute>();

		if (!pRef)
		{
			ImGui::TextDisabled("%s", pId->IsValid() ? pId->ToString().c_str() : "None");
			return false;
		}

		Reflect::Type* pAssetType = ctx.pReflection->GetType(pRef->GetAssetType());

		if (!pAssetType)
		{
			Terminal::Error(StringOps::GetName(this), "{} references an unregistered asset type", field.GetName());
			return false;
		}

		auto* pDomain = ctx.pEngine->RequestService<DomainService>();
		return m_picker.Draw(pId, pAssetType->GetName(), pDomain);
	}
}