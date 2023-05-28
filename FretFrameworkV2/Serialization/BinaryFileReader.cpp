#include "BinaryFileReader.h"

BinaryFileReader::BinaryFileReader(const std::filesystem::path& path) : FileReader(path) { m_ends.push_back(m_file.end()); }
BinaryFileReader::BinaryFileReader(const LoadedFile& file) : FileReader(file) { m_ends.push_back(m_file.end()); }

void BinaryFileReader::enterSection(uint64_t length)
{
	const char* const end = m_currentPosition + length;
	if (end > m_ends.back())
		throw std::runtime_error("Invalid length for section");
	m_ends.push_back(end);
}

void BinaryFileReader::exitSection()
{
	m_currentPosition = m_ends.back();
	m_ends.pop_back();
}

[[nodiscard]] bool BinaryFileReader::checkTag(const char(&tag)[5])
{
	if (strncmp(m_currentPosition, tag, 4) != 0)
		return false;
	return move(4);
}

bool BinaryFileReader::move(size_t amount)
{
	if (m_currentPosition + amount > m_ends.back())
		return false;

	m_currentPosition += amount;
	return true;
}
