#include "SceneAsset.h"

namespace Horizon
{
	bool SceneAsset::Load()
	{
		if (m_loaded)
			return true;

		// Serialization should be here. For now, we just create an empty scene.

		m_loaded = true;
		return true;
	}

	void SceneAsset::Unload()
	{
		m_sceneRegistry.clear();
		m_loaded = false;
	}
}