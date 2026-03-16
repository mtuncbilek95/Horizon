#include "CameraComponentSystem.h"

#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>
#include <Engine/Graphics/GraphicsSystem.h>

#include <imgui.h>

#include <print>

namespace Horizon
{
	struct PushData
	{
		Math::Mat4f model;
		Math::Mat4f viewProj;
		Math::Vec4f cameraPos;
		float lodDistance0;
		float lodDistance1;
		float lodDistance2;
		float padding;
	};

	Math::Vec3f cameraPos = { 0.0f, 0.0f, 0.0f };
	bool moveBack = false;

	SystemReport CameraComponentSystem::OnStart()
	{
		return SystemReport();
	}

	void CameraComponentSystem::OnTick()
	{
		static glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, 0.006f, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, 0.008f, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, 0.003f, glm::vec3(0.0f, 0.0f, 1.0f));

		if (cameraPos.y > 35.0f) 
			moveBack = false;

		if (cameraPos.y < -35.0f) 
			moveBack = true;

		cameraPos.y += moveBack ? 0.01f : -0.01f;

		glm::mat4 view = glm::lookAt(
			cameraPos,
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, -1.0f)
		);
		glm::mat4 proj = glm::perspective(glm::radians(45.0f),
			1920.f / 1080.f,
			0.1f, 100.0f);
		proj[1][1] *= -1;
		glm::mat4 viewProj = proj * view;

		PushData push = {};
		push.model = model;
		push.viewProj = viewProj;
		push.cameraPos = Math::Vec4f(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f);
		push.lodDistance0 = 5.0f;
		push.lodDistance1 = 15.0f;
		push.lodDistance2 = 30.0f;

		auto& graphicsSystem = RequestSystem<GraphicsSystem>();

		graphicsSystem.Submit({[push](GfxCommandBuffer* cmd)
			{
				cmd->BindPushConstants(ShaderStage::Mesh | ShaderStage::Task, 0, sizeof(PushData), &push);
			}});
	}

	void CameraComponentSystem::OnStop()
	{
	}
}