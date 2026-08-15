#include <Runtime/PAL/Watcher/DirectoryWatcher.h>

#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Win32/Helpers/Win32ErrorHelpers.h>

#include <Windows.h>

#include <utility>

namespace Horizon::PAL
{
	namespace
	{
		constexpr DWORD NotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | 
			FILE_NOTIFY_CHANGE_LAST_WRITE;
		constexpr DWORD NotifyBufferSize = 64 * 1024;

		struct WatcherContext
		{
			HANDLE hDirectory = INVALID_HANDLE_VALUE;
			OVERLAPPED overlapped = {};
			BOOL recursive = FALSE;
			b8 armed = false;
			b8 hasPendingRename = false;
			std::string pendingRename;
			alignas(sizeof(DWORD)) u8 notifyBuffer[NotifyBufferSize] = {};
			alignas(sizeof(DWORD)) u8 parseBuffer[NotifyBufferSize] = {};

		};

		std::string ToUtf8(const WCHAR* pData, usize count)
		{
			if (count == 0)
				return std::string();

			i32 length = WideCharToMultiByte(CP_UTF8, 0, pData, (i32)count, nullptr, 0, nullptr, nullptr);

			if (length <= 0)
				return std::string();

			std::string result((usize)length, '\0');
			WideCharToMultiByte(CP_UTF8, 0, pData, (i32)count, result.data(), length, nullptr, nullptr);

			for (c8& character : result)
			{
				if (character == '\\')
					character = '/';
			}

			return result;
		}

		b8 IsDirectoryPath(const std::string& path)
		{
			DWORD attributes = ::GetFileAttributesA(path.data());

			if (attributes == INVALID_FILE_ATTRIBUTES)
				return false;

			return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		}

		void DestroyContext(WatcherContext* pContext)
		{
			if (pContext == nullptr)
				return;

			if (pContext->hDirectory != INVALID_HANDLE_VALUE)
			{
				CancelIoEx(pContext->hDirectory, &pContext->overlapped);

				if (pContext->armed)
				{
					DWORD bytes = 0;
					GetOverlappedResult(pContext->hDirectory, &pContext->overlapped, &bytes, TRUE);
				}

				CloseHandle(pContext->hDirectory);
			}

			if (pContext->overlapped.hEvent != nullptr)
				CloseHandle(pContext->overlapped.hEvent);

			Memory::Allocator::Delete(pContext);
		}

		b8 ArmRead(WatcherContext* pContext, const std::string& rootPath)
		{
			ResetEvent(pContext->overlapped.hEvent);

			pContext->overlapped.Offset = 0;
			pContext->overlapped.OffsetHigh = 0;
			pContext->armed = false;

			DWORD ignored = 0;
			BOOL result = ReadDirectoryChangesW(pContext->hDirectory, pContext->notifyBuffer, NotifyBufferSize,
				pContext->recursive, NotifyFilter, &ignored, &pContext->overlapped, nullptr);

			if (!result)
			{
				Terminal::Error("DirectoryWatcher", "{} cannot be armed: {}", rootPath,
					Win32ErrorHelpers::GetLastErrorString(::GetLastError()));
				return false;
			}

			pContext->armed = true;
			return true;
		}

		void PushOverflow(List<DirectoryWatcher::Event>& outEvents)
		{
			DirectoryWatcher::Event overflow;
			overflow.action = WatcherAction::Overflow;
			overflow.isDirectory = true;

			outEvents.PushBack(overflow);
		}

		void DecodeNotifications(WatcherContext* pContext, const std::string& rootPath, DWORD bytes, List<DirectoryWatcher::Event>& outEvents)
		{
			DWORD offset = 0;

			while (offset + sizeof(FILE_NOTIFY_INFORMATION) <= bytes)
			{
				const FILE_NOTIFY_INFORMATION* pInfo = (const FILE_NOTIFY_INFORMATION*)(pContext->parseBuffer + offset);

				std::string relative = ToUtf8(pInfo->FileName, pInfo->FileNameLength / sizeof(WCHAR));
				std::string absolute = rootPath + "\\" + relative;

				DirectoryWatcher::Event event;
				event.path = absolute;
				event.path = relative;
				event.isDirectory = IsDirectoryPath(absolute);

				switch (pInfo->Action)
				{
				case FILE_ACTION_ADDED:
				{
					event.action = WatcherAction::Added;
					outEvents.PushBack(std::move(event));
					break;
				}
				case FILE_ACTION_REMOVED:
				{
					event.action = WatcherAction::Removed;
					outEvents.PushBack(std::move(event));
					break;
				}
				case FILE_ACTION_MODIFIED:
				{
					event.action = WatcherAction::Modified;
					outEvents.PushBack(std::move(event));
					break;
				}
				case FILE_ACTION_RENAMED_OLD_NAME:
				{
					pContext->pendingRename = std::move(absolute);
					pContext->hasPendingRename = true;
					break;
				}
				case FILE_ACTION_RENAMED_NEW_NAME:
				{
					if (pContext->hasPendingRename)
					{
						event.action = WatcherAction::Renamed;
						event.oldPath = std::move(pContext->pendingRename);
						pContext->hasPendingRename = false;
					}
					else
					{
						event.action = WatcherAction::Added;
					}

					outEvents.PushBack(std::move(event));
					break;
				}
				default:
				{
					Terminal::Debug("DirectoryWatcher", "{} reported an unhandled action {}", absolute, (u32)pInfo->Action);
					break;
				}
				}

				if (pInfo->NextEntryOffset == 0)
					break;

				offset += pInfo->NextEntryOffset;
			}
		}
	}

	DirectoryWatcher::DirectoryWatcher(const std::string& rootPath, b8 recursive) : m_rootPath(rootPath)
	{
		HANDLE hDirectory = ::CreateFileA(m_rootPath.data(), FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);

		if (hDirectory == INVALID_HANDLE_VALUE)
		{
			Terminal::Error("DirectoryWatcher", "{} cannot be opened for watching: {}", m_rootPath,
				Win32ErrorHelpers::GetLastErrorString(GetLastError()));
			return;
		}

		WatcherContext* pContext = Memory::Allocator::Create<WatcherContext>(Memory::CurrLoc());
		pContext->hDirectory = hDirectory;
		pContext->recursive = recursive ? TRUE : FALSE;
		pContext->overlapped.hEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr);

		if (pContext->overlapped.hEvent == nullptr)
		{
			Terminal::Error("DirectoryWatcher", "{} has no completion event: {}", 
				m_rootPath, Win32ErrorHelpers::GetLastErrorString(GetLastError()));

			DestroyContext(pContext);
			return;
		}

		if (!ArmRead(pContext, m_rootPath))
		{
			DestroyContext(pContext);
			return;
		}

		m_handle = pContext;
	}

	DirectoryWatcher::~DirectoryWatcher()
	{
		DestroyContext((WatcherContext*)m_handle);
		m_handle = nullptr;
	}

	DirectoryWatcher::DirectoryWatcher(DirectoryWatcher&& other) noexcept : m_handle(other.m_handle), 
		m_rootPath(std::move(other.m_rootPath))
	{
		other.m_handle = nullptr;
	}

	DirectoryWatcher& DirectoryWatcher::operator=(DirectoryWatcher&& other) noexcept
	{
		if (this != &other)
		{
			DestroyContext((WatcherContext*)m_handle);

			m_handle = other.m_handle;
			m_rootPath = std::move(other.m_rootPath);

			other.m_handle = nullptr;
		}

		return *this;
	}

	b8 DirectoryWatcher::Poll(List<Event>& outEvents)
	{
		if (m_handle == nullptr)
			return false;

		WatcherContext* pContext = (WatcherContext*)m_handle;

		if (!pContext->armed && !ArmRead(pContext, m_rootPath))
			return false;

		DWORD bytes = 0;

		if (!GetOverlappedResult(pContext->hDirectory, &pContext->overlapped, &bytes, FALSE))
		{
			DWORD error = GetLastError();

			if (error == ERROR_IO_INCOMPLETE)
				return true;

			pContext->armed = false;

			if (error == ERROR_NOTIFY_ENUM_DIR)
			{
				Terminal::Warn("DirectoryWatcher", "{} overflowed its notify buffer, a rescan is required", m_rootPath);
				PushOverflow(outEvents);
				return ArmRead(pContext, m_rootPath);
			}

			Terminal::Error("DirectoryWatcher", "{} stopped watching: {}", m_rootPath,
				Win32ErrorHelpers::GetLastErrorString(error));
			return false;
		}

		pContext->armed = false;

		if (bytes == 0 || bytes > NotifyBufferSize)
		{
			Terminal::Warn("DirectoryWatcher", "{} overflowed its notify buffer, a rescan is required", m_rootPath);
			PushOverflow(outEvents);
			return ArmRead(pContext, m_rootPath);
		}

		std::memcpy(pContext->parseBuffer, pContext->notifyBuffer, bytes);

		b8 rearmed = ArmRead(pContext, m_rootPath);
		DecodeNotifications(pContext, m_rootPath, bytes, outEvents);

		return rearmed;
	}
}