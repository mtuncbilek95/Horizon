#include "JsonArchive.h"

namespace Horizon
{
	nlohmann::json& JsonArchiveWriter::NextSlot()
	{
		if (m_stack.IsEmpty())
			return m_root;

		nlohmann::json* current = m_stack.Back();
		if (current->is_array())
		{
			current->push_back(nlohmann::json{});
			return current->back();
		}

		nlohmann::json& slot = (*current)[m_pendingKey];
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
			if (frame.node && frame.node->is_array())
			{
				if (frame.readIndex >= frame.node->size())
				{
					m_hasError = true;
					return nullptr;
				}

				return &frame.node->at(frame.readIndex++);
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

		const nlohmann::json* obj = m_stack.Back().node;
		if (!obj || !obj->is_object())
			return false;

		auto it = obj->find(std::string(name));
		if (it == obj->end())
		{
			m_current = nullptr;
			return false;
		}

		m_current = &(*it);
		return true;
	}

	usize JsonArchiveReader::BeginArray()
	{
		const nlohmann::json* node = Target();
		if (!node || !node->is_array())
		{
			m_hasError = true;
			m_stack.PushBack({ nullptr, 0 });
			return 0;
		}

		usize count = node->size();
		m_stack.PushBack({ node, 0 });
		return count;
	}

	void JsonArchiveReader::EndArray()
	{
		m_stack.PopBack();
	}

	b8 JsonArchiveReader::ReadBool()
	{
		const nlohmann::json* value = Target();
		if (!value || !value->is_boolean())
		{
			m_hasError = true;
			return false;
		}

		return value->get<b8>();
	}

	i64 JsonArchiveReader::ReadI64()
	{
		const nlohmann::json* value = Target();
		if (!value || !value->is_number())
		{
			m_hasError = true;
			return 0;
		}

		return value->get<i64>();
	}

	u64 JsonArchiveReader::ReadU64()
	{
		const nlohmann::json* value = Target();
		if (!value || !value->is_number())
		{
			m_hasError = true;
			return 0;
		}

		return value->get<u64>();
	}

	f64 JsonArchiveReader::ReadF64()
	{
		const nlohmann::json* value = Target();
		if (!value || !value->is_number())
		{
			m_hasError = true;
			return 0.0;
		}

		return value->get<f64>();
	}

	std::string JsonArchiveReader::ReadString()
	{
		const nlohmann::json* value = Target();
		if (!value || !value->is_string())
		{
			m_hasError = true;
			return {};
		}

		return value->get<std::string>();
	}
}