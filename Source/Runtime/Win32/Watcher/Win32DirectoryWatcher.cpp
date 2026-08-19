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
			std::string pendingRelative;
			u32 pendingNameOffset = 0;
			std::wstring rootWide;
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

		std::wstring ToWide(const std::string& value)
		{
			if (value.empty())
				return std::wstring();

			i32 length = MultiByteToWideChar(CP_UTF8, 0, value.data(), (i32)value.size(), nullptr, 0);

			if (length <= 0)
				return std::wstring();

			std::wstring result((usize)length, L'\0');
			MultiByteToWideChar(CP_UTF8, 0, value.data(), (i32)value.size(), result.data(), length);

			return result;
		}

		std::string NormalizeRoot(const std::string& path)
		{
			std::string result = path;

			for (c8& character : result)
			{
				if (character == '\\')
					character = '/';
			}

			while (result.size() > 1 && result.back() == '/')
				result.pop_back();

			return result;
		}

		WatcherEntryKind QueryEntryKind(const std::wstring& rootWide, const WCHAR* pName, usize count)
		{
			std::wstring absolute = rootWide;
			absolute.push_back(L'\\');
			absolute.append(pName, count);

			DWORD attributes = GetFileAttributesW(absolute.data());

			if (attributes == INVALID_FILE_ATTRIBUTES)
				return WatcherEntryKind::Unknown;

			if ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
				return WatcherEntryKind::Directory;

			return WatcherEntryKind::File;
		}

		void ComputeOffsets(const std::string& relative, u32& outNameOffset, u32& outExtensionOffset)
		{
			outNameOffset = 0;
			outExtensionOffset = DirectoryWatcher::Event::NoExtension;

			usize separator = relative.find_last_of('/');

			if (separator != std::string::npos)
				outNameOffset = (u32)(separator + 1);

			usize dot = relative.find_last_of('.');

			if (dot == std::string::npos || dot <= (usize)outNameOffset)
				return;

			outExtensionOffset = (u32)(dot + 1);
		}

		void PushOverflow(WatcherContext* pContext, List<DirectoryWatcher::Event>& outEvents)
		{
			pContext->hasPendingRename = false;
			pContext->pendingRelative.clear();
			pContext->pendingNameOffset = 0;

			DirectoryWatcher::Event overflow;
			overflow.action = WatcherAction::Overflow;
			overflow.kind = WatcherEntryKind::Unknown;

			outEvents.PushBack(std::move(overflow));
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
					Win32ErrorHelpers::GetLastErrorString(GetLastError()));
				return false;
			}

			pContext->armed = true;
			return true;
		}

		void DecodeNotifications(WatcherContext* pContext, const std::string& rootPath, DWORD bytes, List<DirectoryWatcher::Event>& outEvents)
		{
			DWORD offset = 0;

			while (offset + offsetof(FILE_NOTIFY_INFORMATION, FileName) <= bytes)
			{
				const FILE_NOTIFY_INFORMATION* pInfo = (const FILE_NOTIFY_INFORMATION*)(pContext->parseBuffer + offset);

				if (offset + offsetof(FILE_NOTIFY_INFORMATION, FileName) + pInfo->FileNameLength > bytes)
				{
					Terminal::Warn("DirectoryWatcher", "{} produced a truncated notification record", rootPath);
					break;
				}

				const usize nameCount = pInfo->FileNameLength / sizeof(WCHAR);
				std::string relative = ToUtf8(pInfo->FileName, nameCount);

				DirectoryWatcher::Event event;
				event.kind = WatcherEntryKind::Unknown;
				ComputeOffsets(relative, event.nameOffset, event.extensionOffset);

				switch (pInfo->Action)
				{
				case FILE_ACTION_ADDED:
				case FILE_ACTION_MODIFIED:
				{
					event.action = pInfo->Action == FILE_ACTION_ADDED ? WatcherAction::Added : WatcherAction::Modified;
					event.kind = QueryEntryKind(pContext->rootWide, pInfo->FileName, nameCount);
					event.absolutePath = rootPath + "/" + relative;
					event.relativePath = std::move(relative);

					outEvents.PushBack(std::move(event));
					break;
				}
				case FILE_ACTION_REMOVED:
				{
					event.action = WatcherAction::Removed;
					event.absolutePath = rootPath + "/" + relative;
					event.relativePath = std::move(relative);

					outEvents.PushBack(std::move(event));
					break;
				}
				case FILE_ACTION_RENAMED_OLD_NAME:
				{
					pContext->pendingRelative = std::move(relative);
					pContext->pendingNameOffset = event.nameOffset;
					pContext->hasPendingRename = true;
					break;
				}
				case FILE_ACTION_RENAMED_NEW_NAME:
				{
					event.kind = QueryEntryKind(pContext->rootWide, pInfo->FileName, nameCount);
					event.absolutePath = rootPath + "/" + relative;
					event.relativePath = std::move(relative);

					if (pContext->hasPendingRename)
					{
						event.action = WatcherAction::Renamed;
						event.oldRelativePath = std::move(pContext->pendingRelative);
						event.oldNameOffset = pContext->pendingNameOffset;
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
					Terminal::Debug("DirectoryWatcher", "{} reported an unhandled action {}", relative, (u32)pInfo->Action);
					break;
				}
				}

				if (pInfo->NextEntryOffset == 0)
					break;

				offset += pInfo->NextEntryOffset;
			}
		}
	}

	std::string_view DirectoryWatcher::Event::GetParent() const
	{
		return nameOffset == 0 ? std::string_view() : std::string_view(relativePath.data(), nameOffset - 1);
	}

	std::string_view DirectoryWatcher::Event::GetName() const
	{
		return std::string_view(relativePath).substr(nameOffset);
	}

	std::string_view DirectoryWatcher::Event::GetExtension() const
	{
		return extensionOffset == NoExtension ? std::string_view() : std::string_view(relativePath).substr(extensionOffset);
	}

	std::string_view DirectoryWatcher::Event::GetOldParent() const
	{
		return oldNameOffset == 0 ? std::string_view() : std::string_view(oldRelativePath.data(), oldNameOffset - 1);
	}

	std::string_view DirectoryWatcher::Event::GetOldName() const
	{
		return std::string_view(oldRelativePath).substr(oldNameOffset);
	}

	DirectoryWatcher::DirectoryWatcher(const std::string& rootPath, b8 recursive) : m_rootPath(NormalizeRoot(rootPath))
	{
		std::wstring rootWide = ToWide(m_rootPath);

		HANDLE hDirectory = CreateFileW(rootWide.data(), FILE_LIST_DIRECTORY,
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
		pContext->rootWide = std::move(rootWide);
		pContext->overlapped.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);

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
				PushOverflow(pContext, outEvents);
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
			PushOverflow(pContext, outEvents);
			return ArmRead(pContext, m_rootPath);
		}

		std::memcpy(pContext->parseBuffer, pContext->notifyBuffer, bytes);

		b8 rearmed = ArmRead(pContext, m_rootPath);
		DecodeNotifications(pContext, m_rootPath, bytes, outEvents);

		return rearmed;
	}
}