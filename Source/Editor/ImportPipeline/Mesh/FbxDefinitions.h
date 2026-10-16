#pragma once

#include <memory>

struct ufbx_scene;

namespace Horizon::Editor
{
	struct ufbxSceneDeleter
	{
		void operator()(ufbx_scene* pScene) const;
	};

	using ufbxScenePtr = std::unique_ptr<ufbx_scene, ufbxSceneDeleter>;
}