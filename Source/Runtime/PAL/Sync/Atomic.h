#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::PAL
{
	enum class MemoryOrder
	{
		Relaxed,
		Consume,
		Acquire,
		Release,
		AcqRel,
		SeqCst
	};

	struct AtomicOps
	{
		static i64 Load(const i64* pValue, MemoryOrder order = MemoryOrder::SeqCst);
		static i64 FetchStore(i64* pTarget, const i64 value, MemoryOrder order = MemoryOrder::SeqCst);
		static i64 Exchange(i64* pTarget, const i64 value, MemoryOrder order = MemoryOrder::SeqCst);
		static i64 FetchAdd(i64* pTarget, const i64 value, MemoryOrder order = MemoryOrder::SeqCst);
		static i64 FetchSubtract(i64* pTarget, const i64 value, MemoryOrder order = MemoryOrder::SeqCst);
		static i64 Add(i64* pTarget, const i64 value, MemoryOrder order = MemoryOrder::SeqCst);
		static i64 Subtract(i64* pTarget, const i64 value, MemoryOrder order = MemoryOrder::SeqCst);
		static i64 CompareExchange(i64* pTarget, const i64 expectedValue, const i64 desiredValue, MemoryOrder order = MemoryOrder::SeqCst);
	};

	template<typename T>
	class H_EXPORT Atomic
	{
		static_assert(sizeof(T) <= sizeof(i64), "Atomic<T> only supports to types that sizeof(T) == 64.");

	public:
		Atomic() = default;
		Atomic(T value) : m_value(ToRaw(value)) {}

		Atomic(const Atomic&) = delete;
		Atomic& operator=(const Atomic&) = delete;

		T Load(MemoryOrder order = MemoryOrder::SeqCst) const
		{
			return FromRaw(AtomicOps::Load(&m_value, order));
		}

		void Store(T value, MemoryOrder order = MemoryOrder::SeqCst)
		{
			AtomicOps::FetchStore(&m_value, ToRaw(value), order);
		}

		T Exchange(T value, MemoryOrder order = MemoryOrder::SeqCst)
		{
			return FromRaw(AtomicOps::Exchange(&m_value, ToRaw(value), order));
		}

		T FetchAdd(T value, MemoryOrder order = MemoryOrder::SeqCst)
		{
			return FromRaw(AtomicOps::FetchAdd(&m_value, ToRaw(value), order));
		}

		T FetchSubtract(T value, MemoryOrder order = MemoryOrder::SeqCst)
		{
			return FromRaw(AtomicOps::FetchSubtract(&m_value, ToRaw(value), order));
		}

		T Add(T value, MemoryOrder order = MemoryOrder::SeqCst)
		{
			return FromRaw(AtomicOps::Add(&m_value, ToRaw(value), order));
		}

		T Subtract(T value, MemoryOrder order = MemoryOrder::SeqCst)
		{
			return FromRaw(AtomicOps::Subtract(&m_value, ToRaw(value), order));
		}

		T CompareExchange(T expected, T desired, MemoryOrder order = MemoryOrder::SeqCst)
		{
			return FromRaw(AtomicOps::CompareExchange(&m_value, ToRaw(expected), ToRaw(desired), order));
		}

		void* Address() { return (void*)&m_value; }

	private:
		static i64 ToRaw(T value) { return (i64)value; }
		static T FromRaw(i64 raw) { return (T)raw; }

		alignas(sizeof(i64)) mutable i64 m_value = 0;
	};
}