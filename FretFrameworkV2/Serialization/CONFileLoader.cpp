#include "CONFileLoader.h"
#include "Types/UnicodeString.h"

CONFile::CONFileListing::CONFileListing(unsigned char* buf)
{
	m_flags = buf[40];
	m_filename = UnicodeString::strToU32(std::string_view((char*)buf, m_flags & 0x3F));

	memcpy(&m_numBlocks, buf + 41, 3);
	memcpy(&m_firstBlock, buf + 47, 3);
	m_pathIndex = buf[50] << 8 | buf[51];
	m_size = buf[52] << 24 | buf[53] << 16 | buf[54] << 8 | buf[55];
	m_lastWrite = buf[56] << 24 | buf[57] << 16 | buf[58] << 8 | buf[59];
}

void CONFile::CONFileListing::setParentPath(const std::filesystem::path parent)
{
	m_filename = parent / m_filename;
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
	if (strncmp((char*)buffer, "CON ", 4) != 0 &&
		strncmp((char*)buffer, "LIVE", 4) != 0 &&
		strncmp((char*)buffer, "PIRS", 4) != 0)
		throw std::runtime_error("CONFile 4-byte tag invalid or represents a different xbox file type");

	fseek(m_file, 0x0340, SEEK_SET);
	readToBuffer(4);
	m_shiftValue = ((((buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3]) + 0xFFF) & 0xF000) >> 0xC) != 0xB;

	fseek(m_file, 0x37C, SEEK_SET);
	readToBuffer(2);
	const size_t length = 0x1000 * ((size_t)buffer[0] << 8 | (size_t)buffer[1]);

	fseek(m_file, 0x37E, SEEK_SET);
	readToBuffer(3);
	const size_t firstBlock = (size_t)buffer[0] << 16 | (size_t)buffer[1] << 8 | (size_t)buffer[2];

	auto fileListingBuffer = getData_contiguous(firstBlock, length);
	for (size_t i = 0; i < length; i += 0x40)
	{
		CONFileListing listing((unsigned char*)fileListingBuffer.get() + i);
		if (listing.getFilename().empty())
			break;

		if (listing.getPathIndex() != -1)
			listing.setParentPath(m_filelist[listing.getPathIndex()].getFilename());
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
	if (listing.isDirectory())
		throw std::runtime_error("Direcotory listing cannot be loaded as a file");

	if (listing.isContinguous())
		return LoadedFile(getData_contiguous(listing.getFirstBlock(), listing.getFileSize()), listing.getFileSize());
	else
		return LoadedFile(getData_split(listing.getFirstBlock(), listing.getFileSize()), listing.getFileSize());
}

size_t CONFile::CONFileLoader::calculateBlockNum(size_t blocknum) const noexcept
{
	size_t blockAdjust = 0;
	if (blocknum >= 0xAA)
	{
		blockAdjust += ((blocknum / 0xAA) + 1) << m_shiftValue;
		if (blocknum >= 0x70E4)
			blockAdjust += ((blocknum / 0x70E4) + 1) << m_shiftValue;
	}
	return blockAdjust + blocknum;
}

std::shared_ptr<char[]> CONFile::CONFileLoader::getData_contiguous(size_t blockNum, size_t fileSize) const
{
	std::shared_ptr<char[]> data = std::make_shared<char[]>(fileSize + 1);
	if (_fseeki64(m_file, 0xC000 + calculateBlockNum(blockNum) * 0x1000, SEEK_SET) != 0)
		throw;

	const size_t skipVal = (size_t)0x1000 << m_shiftValue;
	size_t div = (blockNum / 28900) + 1;
	size_t numBlocks = 170 - (blockNum % 170);
	size_t readSize = 0x1000 * numBlocks;
	size_t offset = 0;
	while (true)
	{
		if (readSize > fileSize - offset)
			readSize = fileSize - offset;

		if (fread(data.get() + offset, readSize, 1, m_file) != 1)
			throw;

		offset += readSize;
		if (offset == fileSize)
			break;

		blockNum += numBlocks;
		numBlocks = 170;
		readSize = numBlocks * 0x1000;
		_fseeki64(m_file, skipVal, SEEK_CUR);
		if (blockNum == div * 28900)
		{
			_fseeki64(m_file, skipVal, SEEK_CUR);
			++div;
		}
	}
	data[fileSize] = 0;
	return data;
}

std::shared_ptr<char[]> CONFile::CONFileLoader::getData_split(size_t blockNum, size_t fileSize) const
{
	std::shared_ptr<char[]> data = std::make_shared<char[]>(fileSize + 1);
	size_t offset = 0;
	while (true)
	{
		size_t block = calculateBlockNum(blockNum);
		size_t blockLocation = 0xC000 + block * 0x1000;
		if (_fseeki64(m_file, blockLocation, SEEK_SET) != 0)
			throw;

		size_t readSize = 0x1000;
		if (readSize > fileSize - offset)
			readSize = fileSize - offset;

		if (fread(data.get() + offset, readSize, 1, m_file) != 1)
			throw;

		offset += readSize;
		if (offset == fileSize)
			break;

		size_t hashlocation = blockLocation - ((blockNum % 170) * 4072 + 4075);
		if (_fseeki64(m_file, hashlocation, SEEK_SET) != 0)
			throw;

		unsigned char buffer[3];
		if (fread(buffer, 3, 1, m_file) != 1)
			throw;

		blockNum = (size_t)buffer[0] << 16 | (size_t)buffer[1] << 8 | buffer[2];
	}
	data[fileSize] = 0;
	return data;
}
