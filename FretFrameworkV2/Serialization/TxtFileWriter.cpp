#include "TxtFileWriter.h"

TxtFileWriter::TxtFileWriter(const std::filesystem::path& path) : m_file(path, std::ios_base::out | std::ios_base::trunc)
{
	if (!m_file.is_open())
		throw std::runtime_error("Error: " + path.string() + " could not be opened for writing");

	m_file.setf(std::ios_base::boolalpha);
	static const char BOM[4] = { (char)0xEF, (char)0xBB, (char)0xBF, 0 };
	m_file.write(BOM, 3);
}
