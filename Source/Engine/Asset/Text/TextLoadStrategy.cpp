#include "TextLoadStrategy.h"

namespace Horizon::Engine
{
	Asset* TextLoadStrategy::Create(List<u8>&& payload)
	{
		return nullptr;
	}

	void TextLoadStrategy::Destroy(Asset* pAsset)
	{
	}

	b8 TextLoadStrategy::Activate(Asset* pAsset)
	{
		return false;
	}

	void TextLoadStrategy::Deactivate(Asset* pAsset)
	{
	}

	void TextLoadStrategy::OnRegister(Engine* pEngine)
	{
	}

	void TextLoadStrategy::OnUnregister(Engine* pEngine)
	{
	}
}