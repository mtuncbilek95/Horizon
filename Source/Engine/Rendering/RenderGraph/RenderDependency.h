#pragma once

#include <Engine/Rendering/RenderGraph/RenderAccess.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RHI/Texture/TextureDesc.h>
#include <Runtime/RHI/Buffer/BufferDesc.h>

namespace Horizon::Engine
{
	class RenderDependency final
	{
		friend class RenderGraph;

		struct TextureCreate
		{
			Reflect::TypeHandle resource;
			RHI::TextureDesc desc;
		};

		struct BufferCreate
		{
			Reflect::TypeHandle resource;
			RHI::BufferDesc desc;
		};

	public:
		template<typename T>
		void Creates(const RHI::TextureDesc& desc)
		{
			m_textureCreates.PushBack({ Reflect::TypeOf<T>(), desc });
		}

		template<typename T>
		void Creates(const RHI::BufferDesc& desc)
		{
			m_bufferCreates.PushBack({ Reflect::TypeOf<T>(), desc });
		}

		template<typename T>
		void Reads(RHI::ResourceState state)
		{
			m_reads.PushBack({ Reflect::TypeOf<T>(), state });
		}

		template<typename T>
		void Writes(RHI::ResourceState state)
		{
			m_writes.PushBack({ Reflect::TypeOf<T>(), state });
		}

		template<typename... Ts>
		void RunsBefore()
		{
			(m_runsBefore.PushBack(Reflect::TypeOf<Ts>()), ...);
		}

		template<typename... Ts>
		void RunsAfter()
		{
			(m_runsAfter.PushBack(Reflect::TypeOf<Ts>()), ...);
		}

	private:
		List<TextureCreate> m_textureCreates;
		List<BufferCreate> m_bufferCreates;

		List<RenderAccess> m_reads;
		List<RenderAccess> m_writes;

		List<Reflect::TypeHandle> m_runsBefore;
		List<Reflect::TypeHandle> m_runsAfter;
	};
}