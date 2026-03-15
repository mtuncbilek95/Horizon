#include "CameraComponentSystem.h"

#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>
#include <Engine/Graphics/GraphicsSystem.h>

namespace Horizon
{
	struct MeshPushConstants
	{
		alignas(16) Math::Mat4f model;
		alignas(16) Math::Mat4f viewProj;
	};

	SystemReport CameraComponentSystem::OnStart()
	{
		// Log::Terminal(LogType::Success, GetObjectType(), "CameraComponentSystem started.");
		return SystemReport();
	}

	void CameraComponentSystem::OnTick()
	{
		static glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, 0.006f, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 0.008f, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, 0.003f, glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 view = glm::lookAt(
			glm::vec3(0.0f, 30.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, -1.0f)
		);
		glm::mat4 proj = glm::perspective(glm::radians(45.0f),
			1920.f / 1080.f,
			0.1f, 100.0f);
		proj[1][1] *= -1;
		glm::mat4 viewProj = proj * view;

		auto pushData = MeshPushConstants{
			.model = model,
			.viewProj = viewProj
		};

		auto& graphicsSystem = RequestSystem<GraphicsSystem>();

		graphicsSystem.Submit({[pushData](GfxCommandBuffer* cmd)
			{
				cmd->BindPushConstants(ShaderStage::Mesh, 0, sizeof(MeshPushConstants), &pushData);
			}});
	}

	void CameraComponentSystem::OnStop()
	{
	}
}