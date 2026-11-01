#pragma once

#include <Engine/Core/Context.h>
#include <Runtime/Containers/Memory/UniquePtr.h>
#include <Runtime/RHI/Device/Device.h>
#include <Runtime/RHI/Descriptor/DescriptorHeap.h>
#include <Runtime/RHI/Pipeline/PipelineLayout.h>
#include <Runtime/RHI/Swapchain/Swapchain.h>

namespace Horizon::Engine
{
	class ENGINE_API GraphicsContext : public Context
	{
	public:
		static constexpr u32 MaxFramesInFlight = 2;

	public:
		GraphicsContext() = default;
		~GraphicsContext() = default;

		RHI::Device* GetDevice() const { return m_device.Get(); }
		RHI::Swapchain* GetSwapchain() const { return m_swapchain.Get(); }

		RHI::DescriptorHeap* GetResourceHeap() const { return m_resourceHeap.Get(); }
		RHI::DescriptorHeap* GetSamplerHeap() const { return m_samplerHeap.Get(); }

		RHI::PipelineLayout* GetGlobalLayout() const { return m_globalLayout.Get(); }

		ModuleReport OnInitialize() final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleGraph& graph) final;

		void OnLibraryRegistered(const ReflectionLibrary& library) final;
		void OnLibraryUnregistered(const ReflectionLibrary& library) final;

	private:
		UniquePtr<RHI::Device> m_device;

		UniquePtr<RHI::DescriptorHeap> m_resourceHeap;
		UniquePtr<RHI::DescriptorHeap> m_samplerHeap;

		UniquePtr<RHI::PipelineLayout> m_globalLayout;

		UniquePtr<RHI::Swapchain> m_swapchain;
	};
}