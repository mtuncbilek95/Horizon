#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Containers/List.h>
#include <Runtime/Input/Object.h>

namespace Horizon::Input
{
	class Keyboard;
	class Mouse;
	class Gamepad;

	class RUNTIME_API Instance
	{
	public:
		virtual ~Instance();

		virtual void UpdateInputs() = 0;

		Gamepad* GetGamepad(usize index) const;
		Keyboard* GetKeyboard(usize index) const;
		Mouse* GetMouse(usize index) const;
		usize GetObjectCount(DeviceType type) const;

	protected:
		void AddObject(Object* pObject);
		void RemoveObjects(ObjectHandle handle);
		void RemoveAllObjects();

		virtual void ReleaseObject(Object* pObject) = 0;

		const List<Object*>& GetObjects() const { return m_objects; }

	private:
		Object* FindObject(DeviceType type, usize index) const;

	private:
		List<Object*> m_objects;
	};

	RUNTIME_API Instance* CreateInstance();
}