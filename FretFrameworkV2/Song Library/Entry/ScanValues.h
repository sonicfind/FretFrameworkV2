#pragma once
class ScanValues
{
protected:
	unsigned char m_subTracks = 0;
	unsigned char m_intensity = 0xFF;

public:
	ScanValues() = default;
	ScanValues(const ScanValues&) = default;
	ScanValues(ScanValues&&) = default;
	ScanValues& operator=(ScanValues&& scan) = default;

	bool hasSubTrack(size_t sub) const noexcept
	{
		return (1 << sub) & m_subTracks;
	}

	unsigned char getSubTracks() const noexcept { return m_subTracks; }
	unsigned char getIntensity() const noexcept { return m_intensity; }

protected:
	ScanValues& operator=(const ScanValues& scan)
	{
		m_subTracks |= scan.m_subTracks;
		return *this;
	}
};
