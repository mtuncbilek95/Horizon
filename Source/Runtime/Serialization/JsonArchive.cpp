#include "JsonArchive.h"

namespace Horizon
{
	JsonArchive::JsonArchive()
		: m_root(nlohmann::json::object()), m_writing(true), m_valid(true)
	{
		m_stack.push_back({ &m_root, 0 });
	}

	JsonArchive::JsonArchive(const u8* data, usize size) : m_writing(false)
	{
		m_root = nlohmann::json::parse(std::string((const c8*)data, size), nullptr, false);

		if (m_root.is_discarded())
		{
			Terminal::Warn("JsonArchive", "Malformed json payload");
			m_root = nlohmann::json::object();
			m_valid = false;
		}

		m_stack.push_back({ &m_root, 0 });
	}

	std::vector<u8> JsonArchive::ToBytes() const
	{
		std::string text = m_root.dump(2);
		return std::vector<u8>(text.begin(), text.end());
	}

	nlohmann::json& JsonArchive::WriteSlot(const std::string& key)
	{
		Frame& frame = m_stack.back();

		if (key.empty())
		{
			frame.node->push_back(nlohmann::json{});
			return frame.node->back();
		}

		return (*frame.node)[key];
	}

	nlohmann::json* JsonArchive::FindSlot(const std::string& key)
	{
		Frame& frame = m_stack.back();

		if (key.empty())
		{
			if (frame.readIndex >= frame.node->size())
				return nullptr;

			return &frame.node->at(frame.readIndex++);
		}

		auto it = frame.node->find(key);
		if (it == frame.node->end())
		{
			return nullptr;
		}

		return &(*it);
	}

	void JsonArchive::BeginObject(const std::string& key)
	{
		if (m_writing)
		{
			nlohmann::json& node = WriteSlot(key);
			node = nlohmann::json::object();
			m_stack.push_back({ &node, 0 });
			return;
		}

		nlohmann::json* node = FindSlot(key);
		if (!node || !node->is_object())
		{
			m_stack.push_back({ &m_missing, 0 });
			return;
		}

		m_stack.push_back({ node, 0 });
	}

	void JsonArchive::EndObject()
	{
		m_stack.pop_back();
	}

	usize JsonArchive::BeginArray(const std::string& key, usize count)
	{
		if (m_writing)
		{
			nlohmann::json& node = WriteSlot(key);
			node = nlohmann::json::array();
			m_stack.push_back({ &node, 0 });
			return count;
		}

		nlohmann::json* node = FindSlot(key);
		if (!node || !node->is_array())
		{
			m_stack.push_back({ &m_missing, 0 });
			return 0;
		}

		usize size = node->size();
		m_stack.push_back({ node, 0 });
		return size;
	}

	void JsonArchive::EndArray()
	{
		m_stack.pop_back();
	}
}