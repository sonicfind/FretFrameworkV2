#include "CONFileLoader.h"

CONFile::CONFileListing::CONFileListing(unsigned char* buf)
{
	memcpy(m_filename, buf, 0x28);
	m_filename[0x28] = 0;

	buf += 0x28;
	m_flags = buf[0];
	memcpy(&m_numBlocks, buf + 1, 3);
	memcpy(&m_firstBlock, buf + 7, 3);
	m_pathIndex = buf[10] << 8 | buf[11];
	m_size = buf[12] << 24 | buf[13] << 16 | buf[14] << 8 | buf[15];
	m_lastWrite = buf[16] << 24 | buf[17] << 16 | buf[18] << 8 | buf[19];
}

CONFile::CONFileLoader::CONFileLoader(const std::filesystem::path& filepath)
{
	unsigned char buffer[4];
	const auto readToBuffer = [&](size_t count)
	{
		if (fread(buffer, 1, count, m_file) != count)
			throw std::runtime_error("Read error");
	};

	if (fopen_s(&m_file, "testCon", "rb") || !m_file)
		throw std::runtime_error("CONFile could not be loaded");

	readToBuffer(4);
	if (strncmp((char*)buffer, "CON ", 4) != 0)
		throw std::runtime_error("CONFile 4-byte tag invalid or represents a different xbox file type");

	fseek(m_file, 0x0340, SEEK_SET);
	readToBuffer(4);
	m_shiftValue = ((((buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3]) + 0xFFF) & 0xF000) >> 0xC) != 0xB;

	fseek(m_file, 0x37C, SEEK_SET);
	readToBuffer(2);
	const size_t blockCount = (size_t)buffer[0] << 8 | (size_t)buffer[1];
	const size_t length = 0x1000 * blockCount;

	fseek(m_file, 0x37E, SEEK_SET);
	readToBuffer(3);
	const size_t firstBlock = (size_t)buffer[0] << 16 | (size_t)buffer[1] << 8 | (size_t)buffer[2];

	auto fileListingBuffer = getData_contiguous(firstBlock, firstBlock + blockCount, length);
	for (size_t i = 0; i < length; i += 0x40)
	{
		CONFileListing listing((unsigned char*)fileListingBuffer.get() + i);
		if (listing.getFilename().empty())
			break;
		m_filelist.push_back(std::move(listing));
	}
}

CONFile::CONFileLoader::~CONFileLoader()
{
	fclose(m_file);
}

size_t CONFile::CONFileLoader::getFileIndex(std::string_view filename) const noexcept
{
	for (size_t i = 0; i < m_filelist.size(); ++i)
		if (filename == m_filelist[i].getFilename())
			return i;
	return SIZE_MAX;
}

const CONFile::CONFileListing& CONFile::CONFileLoader::getFileListing(std::string_view filename) const
{
	for (size_t i = 0; i < m_filelist.size(); ++i)
		if (filename == m_filelist[i].getFilename())
			return m_filelist[i];
	throw std::runtime_error("Invalid listing filename");
}

const CONFile::CONFileListing& CONFile::CONFileLoader::getFileListing(size_t index) const noexcept
{
	return m_filelist[index];
}

LoadedFile CONFile::CONFileLoader::loadFile(std::string_view filename) const
{
	return loadFile(getFileListing(filename));
}

LoadedFile CONFile::CONFileLoader::loadFile(size_t index) const
{
	return loadFile(m_filelist[index]);
}

LoadedFile CONFile::CONFileLoader::loadFile(const CONFileListing& listing) const
{
	auto data = std::make_unique<char[]>(listing.getFileSize());
	if (!listing.isContinguous())
		throw std::runtime_error("Not yet supported");
	return LoadedFile(getData_contiguous(listing.getFirstBlock(), listing.getFirstBlock() + listing.getNumBlocks(), listing.getFileSize()), listing.getFileSize());
}

size_t CONFile::CONFileLoader::getBlockLocation(size_t blocknum) const noexcept
{
	size_t blockAdjust = 0;
	if (blocknum >= 0xAA)
	{
		blockAdjust += ((blocknum / 0xAA) + 1) << m_shiftValue;
		if (blocknum >= 0x70E4)
			blockAdjust += ((blocknum / 0x70E4) + 1) << m_shiftValue;
	}
	return 0xC000 + (blockAdjust + blocknum) * 0x1000;
}

std::shared_ptr<char[]> CONFile::CONFileLoader::getData_contiguous(size_t blockNum, size_t endBlock, size_t fileSize) const
{
	std::shared_ptr<char[]> data = std::make_shared<char[]>(fileSize);
	size_t readCount = 170 - (blockNum % 170);
	size_t readSize = 0x1000 * readCount;
	size_t offset = 0;
	while (blockNum < endBlock && offset < fileSize)
	{
		if (_fseeki64(m_file, getBlockLocation(blockNum), SEEK_SET) != 0)
			throw;

		if (readSize > fileSize - offset)
			readSize = fileSize - offset;

		if (fread(data.get() + offset, readSize, 1, m_file) != 1)
			throw;

		blockNum += readCount;
		offset += readSize;

		readCount = 170;
		readSize = (size_t)170 * 0x1000;
	}
	return data;
}
