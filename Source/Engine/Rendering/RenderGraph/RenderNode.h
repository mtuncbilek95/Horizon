#pragma once

#include <Engine/World/Blackboard.h>
#include <Runtime/RHI/Queue/QueueType.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::RHI
{
	class CommandList;
	class Texture;
	class TextureView;
	class Buffer;
}

namespace Horizon::Engine
{
	class Engine;
	class RenderDependency;
	class RenderGraph;

	HCLASS();
	class RenderNode : public Reflect::Base
	{
		friend class RenderGraph;
	public:
		virtual b8 OnInitialize() = 0;
		virtual void OnFinalize() = 0;
		virtual void Setup(const Blackboard& blackboard, RenderDependency& graph) = 0;
		virtual void Render(const Blackboard& board) = 0;
		virtual RHI::QueueType GetRunType() const = 0;

		Engine* GetEngine() const { return m_engine; }

	protected:
		RHI::CommandList* GetCommandList() const { return m_cmdList; }

		template<typename T>
		RHI::Texture* GetTexture() const
		{
			return FindTexture(Reflect::TypeOf<T>());
		}

		template<typename T>
		RHI::TextureView* GetTextureView() const
		{
			return FindTextureView(Reflect::TypeOf<T>());
		}

		template<typename T>
		RHI::Buffer* GetBuffer() const
		{
			return FindBuffer(Reflect::TypeOf<T>());
		}

	private:
		RHI::Texture* FindTexture(Reflect::TypeHandle resource) const;
		RHI::TextureView* FindTextureView(Reflect::TypeHandle resource) const;
		RHI::Buffer* FindBuffer(Reflect::TypeHandle resource) const;

	private:
		Engine* m_engine = nullptr;
		RenderGraph* m_graph = nullptr;
		RHI::CommandList* m_cmdList = nullptr;
	};
}