#include "IniFileWriter.h"

void IniFileWriter::writeSection(std::string_view str)
{
	m_file << str << '\n';
}

void IniFileWriter::writeModifiers(const std::vector<Modifiers::Modifier>& list)
{
	for (const Modifiers::Modifier& mod : list)
	{
		m_file << mod.getName() << " =";
		switch (mod.getType())
		{
		case Modifiers::Type::STRING:        write(mod.getValue<UnicodeString>());  break;
		case Modifiers::Type::STRING_NOCASE: write(UnicodeString::U32ToStr(mod.getValue<std::u32string>())); break;
		case Modifiers::Type::UINT32:        write(mod.getValue<uint32_t      >()); break;
		case Modifiers::Type::INT32:         write(mod.getValue<int32_t       >()); break;
		case Modifiers::Type::UINT16:        write(mod.getValue<uint16_t      >()); break;
		case Modifiers::Type::BOOL:          write(mod.getValue<bool          >()); break;
		case Modifiers::Type::FLOAT:         write(mod.getValue<float         >()); break;
		case Modifiers::Type::FLOATARRAY:    write(mod.getValue<float[2]      >()); break;
		}
		m_file << '\n';
	}
}
