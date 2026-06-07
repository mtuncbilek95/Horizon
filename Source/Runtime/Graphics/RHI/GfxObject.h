#pragma once

#include <Runtime/Graphics/RHI/GfxTypes.h>

namespace Horizon
{
	class GfxDevice;
	using GfxHandle = void*;

	class GfxObject
	{
	public:
		GfxObject(GfxDevice* pDevice) : m_device(pDevice) {}
		virtual ~GfxObject() = default;

		GfxHandle GetNative() const { return m_native; }

	protected:
		GfxDevice* m_device = nullptr;
		GfxHandle m_native = nullptr;
	};
}