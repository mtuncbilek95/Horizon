#pragma once

#include <Engine/Core/Context.h>
#include <Runtime/Containers/Memory/UniquePtr.h>
#include <Runtime/RHI/Device/Device.h>
#include <Runtime/RHI/Descriptor/DescriptorHeap.h>
#include <Runtime/RHI/Pipeline/PipelineLayout.h>

namespace Horizon::Engine
{
	class GraphicsContext : public Context
	{
	public:
		static constexpr u32 MaxFramesInFlight = 2;

	public:
		GraphicsContext() = default;
		~GraphicsContext() = default;

		RHI::Device* GetDevice() const { return m_device.Get(); }

		RHI::DescriptorHeap* GetResourceHeap() const { return m_resourceHeap.Get(); }
		RHI::DescriptorHeap* GetSamplerHeap() const { return m_samplerHeap.Get(); }

		RHI::PipelineLayout* GetGlobalLayout() const { return m_globalLayout.Get(); }

		ModuleReport OnInitialize() final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleDependency& graph) final;

	private:
		UniquePtr<RHI::Device> m_device;

		UniquePtr<RHI::DescriptorHeap> m_resourceHeap;
		UniquePtr<RHI::DescriptorHeap> m_samplerHeap;

		UniquePtr<RHI::PipelineLayout> m_globalLayout;
	};
}