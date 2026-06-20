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
		Guid(u32 a, u16 b, u16 c, u64 d);
		Guid(const Guid& other);
		Guid(const std::string& str);
		~Guid();

		u32 GetA() const { return m_a; }
		u32 GetB() const { return m_b; }
		u32 GetC() const { return m_c; }
		u32 GetD() const { return m_d; }

		std::string ToString() const;

		b8 operator==(const Guid& other) const;
		b8 operator!=(const Guid& other) const;

	private:
		u32 m_a;
		u16 m_b;
		u16 m_c;
		u64 m_d;
	};
}

namespace std
{
	template<>
	struct hash<Horizon::Guid>
	{
		size_t operator()(const Horizon::Guid& other) const
		{
			size_t seed = 0;
			auto hashCombine = [](size_t seed, size_t value)
				{
					return seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2));
				};

			seed = hashCombine(seed, hash<uint32_t>{}(other.GetA()));
			seed = hashCombine(seed, hash<uint32_t>{}(other.GetB()));
			seed = hashCombine(seed, hash<uint32_t>{}(other.GetC()));
			seed = hashCombine(seed, hash<uint32_t>{}(other.GetD()));

			return seed;
		}
	};
}