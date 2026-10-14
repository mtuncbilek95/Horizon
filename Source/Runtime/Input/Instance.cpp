#include "Instance.h"

#include <Runtime/Input/Gamepad.h>
#include <Runtime/Input/Keyboard.h>
#include <Runtime/Input/Mouse.h>

#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Input
{
	Instance::~Instance()
	{
		if (m_objects.GetCount() > 0)
			Terminal::Error("Input::Instance", "{} input objects were never removed", m_objects.GetCount());
	}

	Gamepad* Instance::GetGamepad(usize index) const
	{
		return static_cast<Gamepad*>(FindObject(DeviceType::Gamepad, index));
	}

	Keyboard* Instance::GetKeyboard(usize index) const
	{
		return static_cast<Keyboard*>(FindObject(DeviceType::Keyboard, index));
	}

	Mouse* Instance::GetMouse(usize index) const
	{
		return static_cast<Mouse*>(FindObject(DeviceType::Mouse, index));
	}

	usize Instance::GetObjectCount(DeviceType type) const
	{
		usize count = 0;

		for (Object* pObject : m_objects)
		{
			if (pObject->GetDeviceType() == type)
				count++;
		}

		return count;
	}

	void Instance::AddObject(Object* pObject)
	{
		m_objects.PushBack(pObject);
	}

	void Instance::RemoveObjects(ObjectHandle handle)
	{
		for (usize i = m_objects.GetCount(); i > 0; i--)
		{
			Object* pObject = m_objects[i - 1];

			if (pObject->GetHandle() != handle)
				continue;

			ReleaseObject(pObject);
			Memory::Allocator::Delete(pObject);
			m_objects.RemoveAt(i - 1);
		}
	}

	void Instance::RemoveAllObjects()
	{
		for (Object* pObject : m_objects)
		{
			ReleaseObject(pObject);
			Memory::Allocator::Delete(pObject);
		}

		m_objects.Clear();
	}

	Object* Instance::FindObject(DeviceType type, usize index) const
	{
		usize count = 0;

		for (Object* pObject : m_objects)
		{
			if (pObject->GetDeviceType() != type)
				continue;

			if (count == index)
				return pObject;

			count++;
		}

		Terminal::Debug("Input::Instance", "No device of type {} at index {}", u32(type), index);
		return nullptr;
	}
}