#pragma once

#include <Runtime/Input/Instance.h>
#include <Runtime/Containers/List.h>
#include <Runtime/PAL/Sync/CriticalSection.h>

#include <GameInput.h>

namespace Horizon::Input
{
	class Keyboard;
	class Mouse;
	class Gamepad;

	class RUNTIME_API GameInputInstance final : public Instance
	{
		struct PendingDevice
		{
			IGameInputDevice* pDevice = nullptr;
			b8 isConnected = false;
		};

	public:
		~GameInputInstance() final;

		b8 Init();

		void UpdateInputs() final;

		IGameInput* Handle() const { return m_gameInput; }

	private:
		static void CALLBACK OnDeviceStatusChanged(GameInputCallbackToken token, void* pContext, IGameInputDevice* pDevice, 
			u64 timestamp, GameInputDeviceStatus currentStatus, GameInputDeviceStatus previousStatus);

		void ProcessPendingDevices();
		void OnDeviceConnected(IGameInputDevice* pDevice);
		void OnDeviceDisconnected(IGameInputDevice* pDevice);

		void UpdateKeyboard(Keyboard* pKeyboard, IGameInputDevice* pDevice);
		void UpdateMouse(Mouse* pMouse, IGameInputDevice* pDevice);
		void UpdateGamepad(Gamepad* pGamepad, IGameInputDevice* pDevice);

		void ReleaseObject(Object* pObject) final;

	private:
		IGameInput* m_gameInput = nullptr;
		GameInputCallbackToken m_deviceToken = 0;

		PAL::CriticalSection m_pendingLock;
		List<PendingDevice> m_pending;
	};
}