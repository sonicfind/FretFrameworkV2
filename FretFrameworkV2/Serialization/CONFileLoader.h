#pragma once
#include "LoadedFile.h"
namespace CONFile
{


	class CONFileListing
	{
		char m_filename[0x29];
		char m_flags;
		uint32_t m_numBlocks = 0;
		uint32_t m_firstBlock = 0;
		uint16_t m_pathIndex;
		uint32_t m_size;
		int32_t m_lastWrite;

	public:
		CONFileListing(unsigned char* buf);
		std::string_view getFilename() const noexcept { return m_filename; }
		size_t getNumBlocks() const noexcept { return m_numBlocks; }
		size_t getFirstBlock() const noexcept { return m_firstBlock; }
		size_t getPathIndex() const noexcept { return m_pathIndex; }
		size_t getFileSize() const noexcept { return m_size; }
		int32_t getLastWriteTime() const noexcept { return m_lastWrite; }

		bool isDirectory() const noexcept { return m_flags & 0x80; }
		bool isContinguous() const noexcept { return m_flags & 0x40; }
	};

	class CONFileLoader
	{
		uint32_t m_shiftValue;
		FILE* m_file = nullptr;
		std::vector<CONFileListing> m_filelist;
	public:
		CONFileLoader(const std::filesystem::path& filepath);
		~CONFileLoader();
		size_t getFileIndex(std::string_view filename) const noexcept;
		const CONFileListing& getFileListing(std::string_view filename) const;
		const CONFileListing& getFileListing(size_t index) const noexcept;
		LoadedFile loadFile(std::string_view filename) const;
		LoadedFile loadFile(size_t listingIndex) const;
		LoadedFile loadFile(const CONFileListing& listing) const;

	private:
		size_t getBlockLocation(size_t blocknum) const noexcept;
		std::shared_ptr<char[]> getData_contiguous(size_t blockNum, size_t endBlock, size_t fileSize) const;
	};


}


