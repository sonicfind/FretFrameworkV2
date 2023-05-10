#pragma once
#include "BinaryFileReader.h"

class BufferedBinaryReader : public BinaryFileReader<false>
{
public:
	using BinaryFileReader::BinaryFileReader;
	[[nodiscard]] std::string_view extractString();
	void setNextSectionBounds();
	void gotoEndOfBuffer();

protected:
	virtual bool move(uint64_t amount) override;

	[[nodiscard]] std::string_view extractString(uint64_t length);

	template <bool useVirtual = true>
	bool extractWebType(uint64_t& value)
	{
		value = 0;
		if (!extract<uint64_t, useVirtual>(value, 1))
			return false;
		return value < 253 || extract<uint64_t, useVirtual>(value, 2 + 2 * ((value > 253) + 2 * (value > 254)));
	}

	template <bool useVirtual = true>
	[[nodiscard]] uint64_t extractWebType()
	{
		uint64_t value;
		if (!extractWebType<useVirtual>(value))
			throw std::runtime_error("can not parse this data");
		return value;
	}
};

