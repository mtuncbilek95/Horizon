#include <Runtime/GameInput/GameInputInstance.h>

#include <Runtime/Input/Gamepad.h>
#include <Runtime/Input/Keyboard.h>
#include <Runtime/Input/Mouse.h>

#include <Runtime/Containers/ScopedLock.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Win32/Helpers/Win32WindowHelpers.h>

namespace Horizon::Input
{
	namespace
	{
		constexpr u32 kMaxKeys = 16;

		struct ProductEntry
		{
			VendorType vendor;
			u16 productId;
			ProductType product;
		};

		constexpr ProductEntry kProducts[] =
		{
			{ VendorType::Microsoft, 0x02D1, ProductType::XboxOne },
			{ VendorType::Microsoft, 0x02DD, ProductType::XboxOne },
			{ VendorType::Microsoft, 0x02EA, ProductType::XboxOne },
			{ VendorType::Microsoft, 0x0B12, ProductType::XboxSeries },
			{ VendorType::Microsoft, 0x0B13, ProductType::XboxSeries },
			{ VendorType::Sony, 0x05C4, ProductType::Ps4DualShock },
			{ VendorType::Sony, 0x09CC, ProductType::Ps4DualShock },
			{ VendorType::Sony, 0x0CE6, ProductType::Ps5DualSense },
			{ VendorType::Nintendo, 0x2009, ProductType::SwitchPro }
		};

		ProductType ToProductType(VendorType vendor, u16 productId)
		{
			for (const ProductEntry& entry : kProducts)
			{
				if (entry.vendor == vendor && entry.productId == productId)
					return entry.product;
			}

			return ProductType::General;
		}
	}

	Instance* CreateInstance()
	{
		auto* pInstance = Memory::Allocator::Create<GameInputInstance>(Memory::CurrLoc());

		if (!pInstance->Init())
		{
			Memory::Allocator::Delete(pInstance);
			return nullptr;
		}

		return pInstance;
	}

	GameInputInstance::~GameInputInstance()
	{
		if (m_gameInput && m_deviceToken != 0)
			m_gameInput->UnregisterCallback(m_deviceToken, u64_max);

		for (const PendingDevice& pending : m_pending)
			pending.pDevice->Release();

		m_pending.Clear();

		RemoveAllObjects();

		if (m_gameInput)
			m_gameInput->Release();
	}

	b8 GameInputInstance::Init()
	{
		HRESULT hr = GameInputCreate(&m_gameInput);

		if (FAILED(hr))
		{
			Terminal::Error("GameInputInstance", "GameInputCreate failed with 0x{:08X}", u32(hr));
			return false;
		}

		const GameInputKind kinds = GameInputKindKeyboard | GameInputKindMouse | GameInputKindGamepad;
		hr = m_gameInput->RegisterDeviceCallback(nullptr, kinds, GameInputDeviceConnected, GameInputAsyncEnumeration, this, &GameInputInstance::OnDeviceStatusChanged, &m_deviceToken);

		if (FAILED(hr))
		{
			Terminal::Error("GameInputInstance", "RegisterDeviceCallback failed with 0x{:08X}", u32(hr));
			return false;
		}

		return true;
	}

	void GameInputInstance::UpdateInputs()
	{
		ProcessPendingDevices();

		for (Object* pObject : GetObjects())
		{
			auto* pDevice = static_cast<IGameInputDevice*>(pObject->GetHandle());

			switch (pObject->GetDeviceType())
			{
			case DeviceType::Keyboard:
				UpdateKeyboard(static_cast<Keyboard*>(pObject), pDevice);
				break;
			case DeviceType::Mouse:
				UpdateMouse(static_cast<Mouse*>(pObject), pDevice);
				break;
			case DeviceType::Gamepad:
				UpdateGamepad(static_cast<Gamepad*>(pObject), pDevice);
				break;
			default:
				break;
			}
		}
	}

	void CALLBACK GameInputInstance::OnDeviceStatusChanged(GameInputCallbackToken token, void* pContext, IGameInputDevice* pDevice, u64 timestamp, GameInputDeviceStatus currentStatus, GameInputDeviceStatus previousStatus)
	{
		auto* pInstance = static_cast<GameInputInstance*>(pContext);

		const b8 isConnected = (currentStatus & GameInputDeviceConnected) != 0;
		const b8 wasConnected = (previousStatus & GameInputDeviceConnected) != 0;

		if (isConnected == wasConnected)
			return;

		pDevice->AddRef();

		ScopedLock lock(pInstance->m_pendingLock);
		pInstance->m_pending.PushBack({ pDevice, isConnected });
	}

	void GameInputInstance::ProcessPendingDevices()
	{
		List<PendingDevice> pending;

		{
			ScopedLock lock(m_pendingLock);

			for (const PendingDevice& entry : m_pending)
				pending.PushBack(entry);

			m_pending.Clear();
		}

		for (const PendingDevice& entry : pending)
		{
			if (entry.isConnected)
				OnDeviceConnected(entry.pDevice);
			else
				OnDeviceDisconnected(entry.pDevice);

			entry.pDevice->Release();
		}
	}

	void GameInputInstance::OnDeviceConnected(IGameInputDevice* pDevice)
	{
		const GameInputDeviceInfo* pInfo = pDevice->GetDeviceInfo();

		const VendorType vendor = VendorType(pInfo->vendorId);
		const ProductType product = ToProductType(vendor, pInfo->productId);

		if (pInfo->supportedInput & GameInputKindKeyboard)
		{
			pDevice->AddRef();
			AddObject(Memory::Allocator::Create<Keyboard>(Memory::CurrLoc(), this, pDevice, vendor, product));
			Terminal::Info("GameInputInstance", "Keyboard connected, vendor 0x{:04X} product 0x{:04X}", pInfo->vendorId, pInfo->productId);
		}

		if (pInfo->supportedInput & GameInputKindMouse)
		{
			pDevice->AddRef();
			AddObject(Memory::Allocator::Create<Mouse>(Memory::CurrLoc(), this, pDevice, vendor, product));
			Terminal::Info("GameInputInstance", "Mouse connected, vendor 0x{:04X} product 0x{:04X}", pInfo->vendorId, pInfo->productId);
		}

		if (pInfo->supportedInput & GameInputKindGamepad)
		{
			pDevice->AddRef();
			AddObject(Memory::Allocator::Create<Gamepad>(Memory::CurrLoc(), this, pDevice, vendor, product));
			Terminal::Info("GameInputInstance", "Gamepad connected, vendor 0x{:04X} product 0x{:04X}", pInfo->vendorId, pInfo->productId);
		}
	}

	void GameInputInstance::OnDeviceDisconnected(IGameInputDevice* pDevice)
	{
		const GameInputDeviceInfo* pInfo = pDevice->GetDeviceInfo();

		RemoveObjects(pDevice);
		Terminal::Info("GameInputInstance", "Device disconnected, vendor 0x{:04X} product 0x{:04X}", pInfo->vendorId, pInfo->productId);
	}

	void GameInputInstance::UpdateKeyboard(Keyboard* pKeyboard, IGameInputDevice* pDevice)
	{
		KeyboardState state;
		IGameInputReading* pReading = nullptr;

		if (SUCCEEDED(m_gameInput->GetCurrentReading(GameInputKindKeyboard, pDevice, &pReading)))
		{
			GameInputKeyState keys[kMaxKeys];
			const u32 count = pReading->GetKeyState(kMaxKeys, keys);

			for (u32 i = 0; i < count; i++)
			{
				const u64 key = u64(PAL::WindowHelpers::ToWinKey(keys[i].virtualKey));

				if (key != 0 && key < state.size())
					state.set(key);
			}

			pReading->Release();
		}

		pKeyboard->PushState(state);
	}

	void GameInputInstance::UpdateMouse(Mouse* pMouse, IGameInputDevice* pDevice)
	{
		MouseState state = pMouse->GetState();
		IGameInputReading* pReading = nullptr;

		if (SUCCEEDED(m_gameInput->GetCurrentReading(GameInputKindMouse, pDevice, &pReading)))
		{
			GameInputMouseState raw = {};

			if (pReading->GetMouseState(&raw))
			{
				state.buttons = u32(raw.buttons);
				state.positionX = raw.positionX;
				state.positionY = raw.positionY;
				state.wheelX = raw.wheelX;
				state.wheelY = raw.wheelY;
			}

			pReading->Release();
		}

		pMouse->PushState(state);
	}

	void GameInputInstance::UpdateGamepad(Gamepad* pGamepad, IGameInputDevice* pDevice)
	{
		GamepadState state;
		IGameInputReading* pReading = nullptr;

		if (SUCCEEDED(m_gameInput->GetCurrentReading(GameInputKindGamepad, pDevice, &pReading)))
		{
			GameInputGamepadState raw = {};

			if (pReading->GetGamepadState(&raw))
			{
				state.buttons = GamepadButtons(raw.buttons);
				state.leftTrigger = raw.leftTrigger;
				state.rightTrigger = raw.rightTrigger;
				state.leftX = raw.leftThumbstickX;
				state.leftY = raw.leftThumbstickY;
				state.rightX = raw.rightThumbstickX;
				state.rightY = raw.rightThumbstickY;
			}

			pReading->Release();
		}

		pGamepad->PushState(state);
	}

	void GameInputInstance::ReleaseObject(Object* pObject)
	{
		static_cast<IGameInputDevice*>(pObject->GetHandle())->Release();
	}
}