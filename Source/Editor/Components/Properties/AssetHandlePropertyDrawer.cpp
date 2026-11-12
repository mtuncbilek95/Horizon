#include "AssetHandlePropertyDrawer.h"

#include <Editor/Domain/DomainService.h>

#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/Asset/AssetHandle.h>

#include <Runtime/Containers/StringHelper.h>
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
			Terminal::Error(StringHelper::GetTypeName(this), "{} references an unregistered asset type", field.GetName());
			return false;
		}

		auto* pDomain = ctx.pEngine->RequestService<DomainService>();

		if (!m_picker.Draw(pId, pAssetType, pDomain))
			return false;

		static_cast<Engine::AssetHandle<Engine::AssetObject>*>(pValue)->Invalidate();
		return true;
	}
}