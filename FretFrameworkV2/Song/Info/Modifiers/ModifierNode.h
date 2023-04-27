#pragma once
#include "Serialization/TxtFileReader.h"
#include "Modifiers.h"

struct ModifierNode
{
	std::string_view m_name;
	enum Type
	{
		STRING,
		STRING_NOCASE,
		STRING_CHART,
		STRING_CHART_NOCASE,
		UINT32,
		INT32,
		UINT16,
		BOOL,
		FLOAT,
		FLOATARRAY
	} m_type;

	Modifiers::Modifier createModifier(TxtFileReader& reader) const
	{
		Modifiers::Modifier mod(m_name);
		switch (m_type)
		{
		case STRING:
			mod.set(UnicodeString(reader.extractText()));
			break;
		case STRING_NOCASE:
			mod.set(UnicodeString::strToU32(reader.extractText()));
			break;
		case STRING_CHART:
			mod.set(UnicodeString(reader.extractText(false)));
			break;
		case STRING_CHART_NOCASE:
			mod.set(UnicodeString::strToU32(reader.extractText(false)));
			break;
		case UINT32:
			mod.set(reader.extract<uint32_t>());
			break;
		case INT32:
			mod.set(reader.extract<int32_t>());
			break;
		case UINT16:
			mod.set(reader.extract<uint16_t>());
			break;
		case BOOL:
			mod.set(reader.extract<bool>());
			break;
		case FLOAT:
			mod.set(reader.extract<float>());
			break;
		case FLOATARRAY:
		{
			float flt1 = reader.extract<float>();
			float flt2 = reader.extract<float>();
			mod.set(flt1, flt2);
			break;
		}
		}
		return mod;
	}
};
