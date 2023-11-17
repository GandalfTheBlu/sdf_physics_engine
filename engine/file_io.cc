#include "file_io.h"
#include "debug.h"
#include <fstream>
#include <sstream>

namespace Engine
{
	void ReadTextFile(const std::string& path, std::string& text)
	{
		std::ifstream file;
		file.open(path);

		Affirm(file.is_open(), "failed to open file '", path, "'");

		std::stringstream stringStream;
		stringStream << file.rdbuf();
		text = stringStream.str();
		file.close();
	}

	void WriteTextFile(const std::string& path, const std::string& text, bool append)
	{
		std::ofstream file;
		if (append)
			file.open(path, std::ios::app);
		else
			file.open(path);

		file << text;

		file.close();
	}
}