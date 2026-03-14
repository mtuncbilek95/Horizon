#pragma once

namespace Horizon
{
	enum class BufferUsage
	{
		TransferSrc = 1 << 0,
		TransferDst = 1 << 1,
		UniformTexel = 1 << 2,
		StorageTexel = 1 << 3,
		Uniform = 1 << 4,
		Storage = 1 << 5,
		Index = 1 << 6,
		Vertex = 1 << 7,
		Indirect = 1 << 8,
		VideoDecodeSrc = 1 << 9,
		VideoDecodeDst = 1 << 10,
		ShaderDeviceAddress = 1 << 11,
		ResourceDescriptor = 1 << 12,
		SamplerDescriptor = 1 << 13,
		Max = 1 << 14
	};
}
