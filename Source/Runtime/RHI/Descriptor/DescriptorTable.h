#pragma once

#include <Runtime/RHI/Descriptor/DescriptorTableDesc.h>
#include <Runtime/RHI/Object/Object.h>

namespace Horizon::RHI
{
	class TextureView;
	class Buffer;
	class Sampler;

	class DescriptorTable : public Object
	{
	public:
		virtual void Write(u32 binding, u32 arrayIndex, TextureView* pView) = 0;
		virtual void Write(u32 binding, u32 arrayIndex, Buffer* pBuffer, u64 offset, u64 size, u32 stride = 0) = 0;
		virtual void Write(u32 binding, u32 arrayIndex, Sampler* pSampler) = 0;

		DescriptorHeap* GetHeap() const { return m_heap; }
		PipelineLayout* GetLayout() const { return m_layout; }

		u32 GetTableIndex() const { return m_tableIndex; }
		u32 GetVariableCount() const { return m_variableCount; }

	protected:
		DescriptorHeap* m_heap = nullptr;
		PipelineLayout* m_layout = nullptr;

		u32 m_tableIndex = 0;
		u32 m_variableCount = 0;
	};
}