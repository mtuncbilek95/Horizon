#include "JsonArchive.h"

namespace Horizon
{
	nlohmann::json& JsonArchiveWriter::NextSlot()
	{
		if (m_stack.IsEmpty())
			return m_root;

		nlohmann::json* pCurrent = m_stack.Back();
		if (pCurrent->is_array())
		{
			pCurrent->push_back(nlohmann::json{});
			return pCurrent->back();
		}

		nlohmann::json& slot = (*pCurrent)[m_pendingKey];
		m_pendingKey.clear();
		return slot;
	}

	void JsonArchiveWriter::BeginObject()
	{
		nlohmann::json& slot = NextSlot();
		slot = nlohmann::json::object();
		m_stack.PushBack(&slot);
	}

	void JsonArchiveWriter::EndObject()
	{
		m_stack.PopBack();
	}

	void JsonArchiveWriter::Key(std::string_view name)
	{
		m_pendingKey.assign(name);
	}

	void JsonArchiveWriter::BeginArray(usize count)
	{
		(void)count;
		nlohmann::json& slot = NextSlot();
		slot = nlohmann::json::array();
		m_stack.PushBack(&slot);
	}

	void JsonArchiveWriter::EndArray()
	{
		m_stack.PopBack();
	}

	void JsonArchiveWriter::WriteBool(b8 value) { NextSlot() = value; }
	void JsonArchiveWriter::WriteI64(i64 value) { NextSlot() = value; }
	void JsonArchiveWriter::WriteU64(u64 value) { NextSlot() = value; }
	void JsonArchiveWriter::WriteF64(f64 value) { NextSlot() = value; }
	void JsonArchiveWriter::WriteString(std::string_view value) { NextSlot() = std::string(value); }

	std::string JsonArchiveWriter::ToString() const
	{
		return m_root.dump(2);
	}

	List<u8> JsonArchiveWriter::ToBytes() const
	{
		std::string text = m_root.dump(2);

		List<u8> bytes(text.size());
		std::memcpy(bytes.GetData(), text.data(), text.size());
		return bytes;
	}

	JsonArchiveReader::JsonArchiveReader(std::string_view text)
	{
		m_root = nlohmann::json::parse(text.begin(), text.end(), nullptr, false);
		if (m_root.is_discarded())
		{
			m_root = nlohmann::json::object();
			m_hasError = true;
		}

		m_current = &m_root;
	}

	const nlohmann::json* JsonArchiveReader::Target()
	{
		if (!m_stack.IsEmpty())
		{
			Frame& frame = m_stack.Back();
			if (frame.pNode && frame.pNode->is_array())
			{
				if (frame.readIndex >= frame.pNode->size())
				{
					m_hasError = true;
					return nullptr;
				}

				return &frame.pNode->at(frame.readIndex++);
			}
		}

		return m_current;
	}

	void JsonArchiveReader::BeginObject()
	{
		m_stack.PushBack({ Target(), 0 });
	}

	void JsonArchiveReader::EndObject()
	{
		m_stack.PopBack();
	}

	b8 JsonArchiveReader::Key(std::string_view name)
	{
		if (m_stack.IsEmpty())
			return false;

		const nlohmann::json* pObject = m_stack.Back().pNode;
		if (!pObject || !pObject->is_object())
			return false;

		auto it = pObject->find(std::string(name));
		if (it == pObject->end())
		{
			m_current = nullptr;
			return false;
		}

		m_current = &(*it);
		return true;
	}

	usize JsonArchiveReader::BeginArray()
	{
		const nlohmann::json* pNode = Target();
		if (!pNode || !pNode->is_array())
		{
			m_hasError = true;
			m_stack.PushBack({ nullptr, 0 });
			return 0;
		}

		usize count = pNode->size();
		m_stack.PushBack({ pNode, 0 });
		return count;
	}

	void JsonArchiveReader::EndArray()
	{
		m_stack.PopBack();
	}

	b8 JsonArchiveReader::ReadBool()
	{
		const nlohmann::json* pValue = Target();
		if (!pValue || !pValue->is_boolean())
		{
			m_hasError = true;
			return false;
		}

		return pValue->get<b8>();
	}

	i64 JsonArchiveReader::ReadI64()
	{
		const nlohmann::json* pValue = Target();
		if (!pValue || !pValue->is_number())
		{
			m_hasError = true;
			return 0;
		}

		return pValue->get<i64>();
	}

	u64 JsonArchiveReader::ReadU64()
	{
		const nlohmann::json* pValue = Target();
		if (!pValue || !pValue->is_number())
		{
			m_hasError = true;
			return 0;
		}

		return pValue->get<u64>();
	}

	f64 JsonArchiveReader::ReadF64()
	{
		const nlohmann::json* pValue = Target();
		if (!pValue || !pValue->is_number())
		{
			m_hasError = true;
			return 0.0;
		}

		return pValue->get<f64>();
	}

	std::string JsonArchiveReader::ReadString()
	{
		const nlohmann::json* pValue = Target();
		if (!pValue || !pValue->is_string())
		{
			m_hasError = true;
			return {};
		}

		return pValue->get<std::string>();
	}
}