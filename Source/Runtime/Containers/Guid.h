#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <xhash>
#include <string>

namespace Horizon
{
	class Guid final
	{
	public:
		static Guid Generate();

	public:
		Guid();
		Guid(u32 a, u16 b, u16 c, const u8(&d)[8]);
		explicit Guid(const std::string& str);

		Guid(const Guid& other) = default;
		Guid& operator=(const Guid& other) = default;
		~Guid() = default;

		b8 IsValid() const;

		u32 GetA() const { return m_a; }
		u16 GetB() const { return m_b; }
		u16 GetC() const { return m_c; }
		const u8* GetD() const { return m_d; }

		std::string ToString() const;

		b8 operator==(const Guid& other) const;
		b8 operator!=(const Guid& other) const;

	private:
		u32 m_a;
		u16 m_b;
		u16 m_c;
		u8 m_d[8];
	};
}

namespace std
{
	template<>
	struct hash<Horizon::Guid>
	{
		size_t operator()(const Horizon::Guid& other) const noexcept
		{
			auto hashCombine = [](size_t seed, size_t value)
				{
					return seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2));
				};

			size_t seed = 0;
			seed = hashCombine(seed, hash<uint32_t>{}(other.GetA()));
			seed = hashCombine(seed, hash<uint16_t>{}(other.GetB()));
			seed = hashCombine(seed, hash<uint16_t>{}(other.GetC()));

			uint64_t tail = 0;
			for (int i = 0; i < 8; ++i)
				tail = (tail << 8) | other.GetD()[i];

			return hashCombine(seed, hash<uint64_t>{}(tail));
		}
	};
}