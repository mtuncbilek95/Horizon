#pragma once

namespace Horizon::Engine
{
	class RenderDependency;

	class RenderNode : public Reflect::Base
	{
	public:
		virtual void Setup(RenderDependency& graph) = 0;
		virtual void Render() = 0;
	};
}