#include "file_watcher.h"
#include "debug.h"
#include <chrono>
#include <filesystem>

namespace Engine
{
	FileWatcher::FileWatcher() :
		lastChangeTime(0.0)
	{}

	void FileWatcher::Init(const std::string& _filePath)
	{
		filePath = _filePath;
		NewVersionAvailable();
	}

	bool FileWatcher::NewVersionAvailable()
	{
		try
		{
			std::filesystem::path p = filePath;
			std::filesystem::file_time_type writeTime = std::filesystem::last_write_time(p);
			double newChangeTime = std::chrono::duration_cast<std::chrono::seconds>(writeTime.time_since_epoch()).count();
			if (newChangeTime > lastChangeTime)
			{
				lastChangeTime = newChangeTime;
				return true;
			}
		}
		catch (const std::exception& e)
		{
			Affirm(false, "failed to find file '", filePath, "' when file watching");
		}

		return false;
	}
}