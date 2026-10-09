#pragma once

#include <Editor/Domain/ImportPipeline/ImportPipelineInfoAttribute.h>
#include <Editor/Domain/ImportPipeline/ImportPipeline.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(ImportPipelineInfo[{ ".fbx", ".glb", ".gltf" }]);
	class EDITOR_API MeshImportPipeline : public ImportPipeline 
	{
		HORIZON_TYPE_REFLECT(MeshImportPipeline);
	public:
		MeshImportPipeline() = default;
		~MeshImportPipeline() = default;
	};
}