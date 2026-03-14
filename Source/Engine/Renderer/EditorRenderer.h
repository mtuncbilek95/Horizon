#pragma once

#include <Engine/Object/Object.h>
#include <Engine/System/SystemReport.h>

namespace Horizon
{
	class GfxCommandBuffer;
	class GfxDescriptorPool;

	class EditorRenderer : public Object<EditorRenderer>
	{
		using EditorContext = void*;
	public:
		EditorRenderer();
		~EditorRenderer() = default;

		SystemReport Initialize(ISystem* ownerSystem);
		void Render(GfxCommandBuffer* pDrawBuffer);
		void Finalize();

	private:
		EditorContext m_editorContext;
		std::shared_ptr<GfxDescriptorPool> m_imguiDescriptorPool;
	};
}