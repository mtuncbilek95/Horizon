#pragma once

namespace Horizon::RHI
{
	class GfxDevice;

	/**
	 * @brief Base of every device owned RHI resource.
	 * Keeps the device that created the object and lets
	 * the backend attach a debug name to it.
	 *
	 * @code
	 *   myBuffer->SetDebugName("SceneVertexBuffer");
	 *   RHI::GfxDevice* device = myBuffer->GetOwnerDevice();
	 * @endcode
	 */
	class GfxObject
	{
	public:
		virtual ~GfxObject() = default;

		virtual void SetDebugName(const char* pName) {}

		GfxDevice* GetOwnerDevice() const { return m_ownerDevice; }

	protected:
		GfxDevice* m_ownerDevice = nullptr;
	};
}