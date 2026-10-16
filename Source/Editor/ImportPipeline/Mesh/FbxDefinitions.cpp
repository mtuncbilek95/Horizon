#include "FbxDefinitions.h"

#include <ufbx.h>

namespace Horizon::Editor
{
	void ufbxSceneDeleter::operator()(ufbx_scene* pScene) const
	{
		ufbx_free_scene(pScene);
	}
}