#pragma once

#include <string>

namespace Horizon
{
	class Guid final
	{
		friend struct GuidHash;
		friend struct GuidHelper;
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

		b8 IsValid() const;

	private:
		u32 m_a;
		u16 m_b;
		u16 m_c;
		u64 m_d;
	};

	struct GuidHash
	{
		usize operator()(const Guid& other) const;
	};

	struct GuidHelper
	{
		static Guid GenerateID();
	};
}