#pragma once
#include "BinaryFileReader.h"

class BufferedBinaryReader : public BinaryFileReader<false>
{
public:
	using BinaryFileReader::BinaryFileReader;
	[[nodiscard]] std::string_view extractString();
	void setNextSectionBounds();

protected:
	virtual bool move(size_t amount) override;

	[[nodiscard]] std::string_view extractString(size_t length);

	template <bool useVirtual = true>
	bool extractWebType(uint32_t& value)
	{
		value = 0;
		if (!extract<uint32_t, useVirtual>(value, 1))
			return false;

		return value < 254 || extract<uint32_t, useVirtual>(value, 2 + 2ULL * (value == 255));
	}

	template <bool useVirtual = true>
	[[nodiscard]] uint32_t extractWebType()
	{
		uint32_t value;
		if (!extractWebType<useVirtual>(value))
			throw std::runtime_error("can not parse this data");
		return value;
	}
};

