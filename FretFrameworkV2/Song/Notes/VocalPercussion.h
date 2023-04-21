#pragma once
#include <vector>

class VocalPercussion
{
	bool m_isPlayable = true;
public:
	bool modify(char modifier);
	bool isPlayable() const;
	void togglePlayable();
	void setPlayable(bool enable);
	std::vector<char> getActiveModifiers() const;
};
