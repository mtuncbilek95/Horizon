#include <Runtime/Window/Window.h>

#include <Runtime/Graphics/RHI/Instance/GfxInstance.h>
#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Swapchain/GfxSwapchain.h>
#include <Runtime/Graphics/RHI/Queue/GfxQueue.h>
#include <Runtime/Graphics/RHI/Sync/GfxFence.h>
#include <Runtime/Graphics/RHI/Sync/GfxSemaphore.h>
#include <Runtime/Graphics/RHI/Command/GfxCommandPool.h>
#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>
#include <Runtime/Graphics/RHI/Shader/GfxShader.h>
#include <Runtime/Graphics/RHI/Pipeline/GfxPipeline.h>
#include <Runtime/Graphics/RHI/Buffer/GfxBuffer.h>
#include <Runtime/ShaderCompiler/ShaderCompiler.h>
#include <Runtime/Data/Geometry.h>

#include <filesystem>

// https://vkguide.dev/
// https://vulkan-tutorial.com/
// Dynamic Rendering


using namespace Horizon;

static const std::filesystem::path gVertexShaderPath = "";
static const std::filesystem::path gFragmentShaderPath = "";

int main(int argc, char* argv[])
{
	auto window = Window(WindowDesc()
		.setWindowMode(WindowMode::Windowed)
		.setWindowName("Test")
		.setWindowSize({ 1920u, 1080u }));
	window.Show();

	// Instance

	// Device

	// GraphicsQueue

	// Swapchain

	while (window.IsActive())
	{
		window.ProcessEvents();
	}
}
