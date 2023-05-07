#pragma once
class ScanValues
{
public:
	unsigned char m_subTracks = 0;
	unsigned char m_intensity = 0xFF;
	virtual ~ScanValues() {}

	void addSubTrack(size_t diff)
	{
		m_subTracks |= 1 << diff;
	}

	[[nodiscard]] bool wasTrackValidated(size_t diff) const noexcept
	{
		return (1 << diff) & m_subTracks;
	}
};
