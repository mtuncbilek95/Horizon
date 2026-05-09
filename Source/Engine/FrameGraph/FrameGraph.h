#pragma once

#include <Runtime/Graphics/RHI/Util/ImageFormat.h>
#include <Runtime/Graphics/RHI/Util/BufferFormat.h>

#include <Engine/FrameGraph/GraphResourceHandle.h>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Horizon
{
	enum class ResourceLifetime
	{
		Transient,
		Persistent
	};

	struct FrameGraphTextureDesc
	{
		std::string name;
		Math::Vec2u size;
		u32 mipLevels = 1;
		ImageFormat format;
		ResourceLifetime lifetime = ResourceLifetime::Transient;

		FrameGraphTextureDesc& setName(const std::string& n) { name = n; return *this; }
		FrameGraphTextureDesc& setSize(Math::Vec2u s) { size = s; return *this; }
		FrameGraphTextureDesc& setMipLevels(u32 mips) { mipLevels = mips; return *this; }
		FrameGraphTextureDesc& setFormat(ImageFormat fmt) { format = fmt; return *this; }
		FrameGraphTextureDesc& setLifetime(ResourceLifetime lt) { lifetime = lt; return *this; }
	};

	struct FrameGraphBufferDesc
	{
		std::string name;
		u64 size = 0;
		BufferUsage usage = BufferUsage::Storage;
		ResourceLifetime lifetime = ResourceLifetime::Transient;

		FrameGraphBufferDesc& setName(const std::string& n) { name = n; return *this; }
		FrameGraphBufferDesc& setSize(u64 s) { size = s; return *this; }
		FrameGraphBufferDesc& setUsage(BufferUsage u) { usage = u; return *this; }
		FrameGraphBufferDesc& setLifetime(ResourceLifetime lt) { lifetime = lt; return *this; }
	};

	class GfxImage;
	class GfxImageView;
	class GfxBuffer;
	class GfxCommandBuffer;

	enum class FrameGraphTextureUsage
	{
		ColorAttachment,
		DepthAttachment,
		ShaderRead,
		StorageRead,
		StorageWrite,
		TransferSrc,
		TransferDst
	};

	enum class FrameGraphBufferUsage
	{
		UniformRead,
		StorageRead,
		StorageWrite,
		IndirectRead,
		TransferSrc,
		TransferDst
	};

	struct FrameGraphTextureAccess
	{
		TextureHandle handle;
		FrameGraphTextureUsage usage;
	};

	struct FrameGraphBufferAccess
	{
		BufferHandle handle;
		FrameGraphBufferUsage usage;
	};

	using PassExecuteFn = std::function<void(GfxCommandBuffer&)>;

	struct RenderPass
	{
		std::string name;

		std::vector<FrameGraphTextureAccess> texReads;
		std::vector<FrameGraphTextureAccess> texWrites;
		std::vector<FrameGraphBufferAccess> bufReads;
		std::vector<FrameGraphBufferAccess> bufWrites;

		PassExecuteFn execute;

		RenderPass& setName(std::string_view n) { name = n; return *this; }
		RenderPass& addTexRead(FrameGraphTextureAccess a) { texReads.push_back(a);  return *this; }
		RenderPass& addTexWrite(FrameGraphTextureAccess a) { texWrites.push_back(a); return *this; }
		RenderPass& addBufRead(FrameGraphBufferAccess a) { bufReads.push_back(a);  return *this; }
		RenderPass& addBufWrite(FrameGraphBufferAccess a) { bufWrites.push_back(a); return *this; }
		RenderPass& setExecute(PassExecuteFn fn) { execute = std::move(fn); return *this; }
	};

	class FrameGraph
	{
	public:
		TextureHandle createTexture(const FrameGraphTextureDesc& desc);
		TextureHandle importTexture(std::string_view name, GfxImageView* image);
		TextureHandle writeTexture(TextureHandle handle);

		BufferHandle createBuffer(const FrameGraphBufferDesc& desc);
		BufferHandle importBuffer(std::string_view name, GfxBuffer* buffer);
		BufferHandle writeBuffer(BufferHandle handle);

		void addPass(RenderPass pass);

		void compile();
		void execute(GfxCommandBuffer& cmd);
		void reset();

		GfxImage* getImage(TextureHandle handle)  const;
		GfxBuffer* getBuffer(BufferHandle handle)  const;

	private:
		struct TextureEntry
		{
			FrameGraphTextureDesc desc;
			u32 version  = 0;
			std::shared_ptr<GfxImage> owned;
			GfxImageView* view = nullptr;
			b8 imported = false;

			GfxImage* getImage() const;
		};

		struct BufferEntry
		{
			FrameGraphBufferDesc desc;
			u32 version = 0;
			std::shared_ptr<GfxBuffer> owned;
			GfxBuffer* raw = nullptr;
			b8 imported = false;

			GfxBuffer* getBuffer() const;
		};

		std::vector<TextureEntry> m_textures;
		std::vector<BufferEntry> m_buffers;
		std::vector<RenderPass> m_passes;

		std::vector<std::vector<usize>> m_adj;
		std::vector<i32> m_inDeg;
		std::vector<usize> m_order;
		std::vector<ImageUsage> m_textureUsages;

		std::unordered_map<std::string, std::shared_ptr<GfxImage>> m_persistentTextures;
		std::unordered_map<std::string, std::shared_ptr<GfxBuffer>> m_persistentBuffers;

		b8 m_compiled = false;
	};
}