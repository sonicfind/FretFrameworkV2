#include "BinaryFileWriter.h"

BinaryFileWriter::BinaryFileWriter(const std::filesystem::path& path) : m_file(path, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary)
{
	if (!m_file.is_open())
		throw std::runtime_error("Error: " + path.string() + " could not be opened for writing");
}

std::streampos BinaryFileWriter::tell()
{
	return m_file.tellp();
}

void BinaryFileWriter::seek(std::streampos pos)
{
	m_file.seekp(pos);
}
