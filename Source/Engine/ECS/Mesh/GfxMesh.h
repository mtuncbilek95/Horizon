#pragma once

namespace Horizon
{
	class GfxBuffer;

	class GfxMesh
	{
	public:
		GfxMesh() = default;
		virtual ~GfxMesh() = default;

		GfxBuffer* VBuffer() const { return m_vertexBuffer.get(); }
		GfxBuffer* IBuffer() const { return m_indexBuffer.get(); }

	private:
		std::shared_ptr<GfxBuffer> m_vertexBuffer;
		std::shared_ptr<GfxBuffer> m_indexBuffer;
	};
}