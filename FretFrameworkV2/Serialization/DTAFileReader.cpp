#include "DTAFileReader.h"

DTA::DTAFileReader::DTAFileReader(const LoadedFile& file) : FileReader(file)
{
	skipWhiteSpace();
}

std::string_view DTA::DTAFileReader::extractText()
{
	bool inSquirley = *m_currentPosition == '{';
	bool inQuotes = !inSquirley && *m_currentPosition == '\"';
	bool inApostrophes = !inQuotes && *m_currentPosition == '\'';
	
	if (inSquirley || inQuotes || inApostrophes)
		++m_currentPosition;

	const char* pos = m_currentPosition++;
	while (m_currentPosition < m_nodeEnds.back())
	{
		const char ch = *m_currentPosition;
		if (ch == '{')
			throw std::runtime_error("Text error - no { braces allowed");

		if (ch == '}')
		{
			if (inSquirley)
				break;
			throw std::runtime_error("Text error - no \'}\' allowed");
		}
		else if (ch == '\"')
		{
			if (inQuotes)
				break;
			if (!inSquirley)
				throw std::runtime_error("Text error - no quotes allowed");
		}
		else if (ch == '\'')
		{
			if (inApostrophes)
				break;
			if (!inSquirley && !inQuotes)
				throw std::runtime_error("Text error - no apostrophes allowed");
		}
		else if (ch <= 32)
		{
			if (inApostrophes)
				throw std::runtime_error("Text error - no whitespace allowed");
			if (!inSquirley && !inQuotes)
				break;
		}
		++m_currentPosition;
	}

	const char* const end = m_currentPosition;
	if (m_currentPosition != m_nodeEnds.back())
	{
		++m_currentPosition;
		skipWhiteSpace();
	}
	else if (inSquirley || inQuotes || inApostrophes)
		throw std::runtime_error("Improper end to text");
	
	return std::string_view(pos, end);
}

void DTA::DTAFileReader::runLoop(const std::function<void()>& func)
{
	while (startNode())
	{
		func();
		endNode();
	}
}

bool DTA::DTAFileReader::startNode()
{
	if (*m_currentPosition != '(')
		return false;

	++m_currentPosition;
	skipWhiteSpace();

	size_t scopeLevel = 1;
	bool inApostropes = false;
	bool inQuotes = false;
	bool inComment = false;
	const char* pos = m_currentPosition;
	while (scopeLevel >= 1 && pos < m_file.end())
	{
		const char ch = *pos;
		if (inComment)
		{
			if (ch == '\n')
				inComment = false;
		}
		else if (ch == '\"')
		{
			if (inApostropes)
				throw std::runtime_error("Ahh hell nah wtf");
			inQuotes = !inQuotes;
		}
		else if (!inQuotes)
		{
			if (!inApostropes)
			{
				if (ch == '(')
					++scopeLevel;
				else if (ch == ')')
					--scopeLevel;
				else if (ch == '\'')
					inApostropes = true;
				else if (ch == ';')
					inComment = true;
			}
			else if (ch == '\'')
				inApostropes = false;
		}
		++pos;
	}
	m_nodeEnds.push_back(pos - 1);
	return true;
}

void DTA::DTAFileReader::endNode()
{
	m_currentPosition = m_nodeEnds.back() + 1;
	m_nodeEnds.pop_back();
	skipWhiteSpace();
}

std::string_view DTA::DTAFileReader::getNameOfNode()
{
	if (*m_currentPosition == '(')
		return {};

	bool hasApostrophe = true;
	if (*m_currentPosition != '\'')
	{
		if (m_currentPosition[-1] != '(')
			throw std::runtime_error("Invalid name call");
		hasApostrophe = false;
	}
	else
		++m_currentPosition;

	const char* const start = m_currentPosition;
	while (*m_currentPosition != '\'')
	{
		if (*m_currentPosition <= 32)
		{
			if (hasApostrophe)
				throw std::runtime_error("Invalid name format");
			break;
		}
		++m_currentPosition;
	}
	const char* const end = m_currentPosition++;
	skipWhiteSpace();
	return std::string_view(start, end);
}

template <>
bool DTA::DTAFileReader::extract(char& value)
{
	if (m_currentPosition >= m_nodeEnds.back())
		return false;

	value = *m_currentPosition++;
	skipWhiteSpace();
	return true;
}

template<>
bool DTA::DTAFileReader::extract(bool& value)
{
	switch (*m_currentPosition)
	{
	case '0':
		value = false; break;
	case '1':
		value = true; break;
	default:
		value = m_currentPosition + 4 <= m_nodeEnds.back() &&
			(m_currentPosition[0] == 't' || m_currentPosition[0] == 'T') &&
			(m_currentPosition[1] == 'r' || m_currentPosition[1] == 'R') &&
			(m_currentPosition[2] == 'u' || m_currentPosition[2] == 'U') &&
			(m_currentPosition[3] == 'e' || m_currentPosition[3] == 'E');
	}
	return true;
}

void DTA::DTAFileReader::skipWhiteSpace()
{
	while (m_currentPosition < m_file.end())
	{
		if (*m_currentPosition <= 32)
			++m_currentPosition;
		else if (*m_currentPosition == ';')
		{
			++m_currentPosition;
			while (m_currentPosition < m_file.end())
			{
				++m_currentPosition;
				if (m_currentPosition[-1] == '\n')
					break;
			}
		}
		else
			break;
	}
}

DTA::DTAFileNode::DTAFileNode(DTAFileReader& reader) : m_nodeName(reader.getNameOfNode())
{
	reader.runLoop([&]{
		std::string_view name = reader.getNameOfNode();
		if (name == "name") m_name = reader.extractText();
		else if (name == "artist") m_artist = reader.extractText();
		else if (name == "master") m_isMaster = reader.extract<bool>();
		else if (name == "context") m_context = reader.extract<uint32_t>();
		else if (name == "song")
		{
			reader.runLoop([this, &reader] {
				std::string_view descriptor = reader.getNameOfNode();
				if (descriptor == "name") m_location = reader.extractText();
				else if (descriptor == "tracks")
				{
					reader.runLoop([this, &reader] {
						reader.runLoop([this, &reader] {
							std::string_view trackname = reader.getNameOfNode();
							if (trackname == "drum") reader.runLoop([this, &reader]{ m_trackIndices.drum = reader.extractList<uint16_t>(); });
							else if (trackname == "bass") reader.runLoop([this, &reader] { m_trackIndices.bass = reader.extractList<uint16_t>(); });
							else if (trackname == "guitar") reader.runLoop([this, &reader] { m_trackIndices.guitar = reader.extractList<uint16_t>(); });
							else if (trackname == "keys") reader.runLoop([this, &reader] { m_trackIndices.keys = reader.extractList<uint16_t>(); });
							else if (trackname == "vocals") reader.runLoop([this, &reader] { m_trackIndices.vocals = reader.extractList<uint16_t>(); });
						});
					});
				}
				else if (descriptor == "crowd_channels") m_trackIndices.crowd = reader.extractList<uint16_t>();
				else if (descriptor == "vocal_parts") m_numVocalParts = reader.extract<uint16_t>();
				else if (descriptor == "pans")
				{
					reader.runLoop([this, &reader] {
						if (m_trackIndices.drum.size() > 2)
						{
							m_audio.kick.pan[0] = reader.extract<float>();
							if (m_trackIndices.drum.size() > 5)
								m_audio.kick.pan[1] = reader.extract<float>();
						}

						if (m_trackIndices.drum.size() > 1)
						{
							m_audio.snare.pan[0] = reader.extract<float>();
							if (m_trackIndices.drum.size() > 4)
								m_audio.snare.pan[1] = reader.extract<float>();
						}

						if (m_trackIndices.drum.size() > 0)
						{
							m_audio.cymbals.pan[0] = reader.extract<float>();
							if (m_trackIndices.drum.size() > 3)
								m_audio.cymbals.pan[1] = reader.extract<float>();
						}

						if (m_trackIndices.bass.size() > 0)
						{
							m_audio.bass.pan[0] = reader.extract<float>();
							if (m_trackIndices.bass.size() > 1)
								m_audio.bass.pan[1] = reader.extract<float>();
						}

						if (m_trackIndices.guitar.size() > 0)
						{
							m_audio.guitar.pan[0] = reader.extract<float>();
							if (m_trackIndices.guitar.size() > 1)
								m_audio.guitar.pan[1] = reader.extract<float>();
						}

						if (m_trackIndices.vocals.size() > 0)
						{
							m_audio.vocals.pan[0] = reader.extract<float>();
							if (m_trackIndices.vocals.size() > 1)
								m_audio.vocals.pan[1] = reader.extract<float>();
						}

						if (!reader.extract(m_audio.tracks.pan[0]) || !reader.extract(m_audio.tracks.pan[1]))
							return;

						if (m_trackIndices.crowd.size() > 0)
						{
							m_audio.crowd.pan[0] = reader.extract<float>();
							if (m_trackIndices.crowd.size() > 1)
								m_audio.crowd.pan[1] = reader.extract<float>();
						}
					});
				}
				else if (descriptor == "vols")
				{
					reader.runLoop([this, &reader] {
						if (m_trackIndices.drum.size() > 2)
						{
							m_audio.kick.volume[0] = reader.extract<float>();
							if (m_trackIndices.drum.size() > 5)
								m_audio.kick.volume[1] = reader.extract<float>();
						}

						if (m_trackIndices.drum.size() > 1)
						{
							m_audio.snare.volume[0] = reader.extract<float>();
							if (m_trackIndices.drum.size() > 4)
								m_audio.snare.volume[1] = reader.extract<float>();
						}

						if (m_trackIndices.drum.size() > 0)
						{
							m_audio.cymbals.volume[0] = reader.extract<float>();
							if (m_trackIndices.drum.size() > 3)
								m_audio.cymbals.volume[1] = reader.extract<float>();
						}

						if (m_trackIndices.bass.size() > 0)
						{
							m_audio.bass.volume[0] = reader.extract<float>();
							if (m_trackIndices.bass.size() > 1)
								m_audio.bass.volume[1] = reader.extract<float>();
						}

						if (m_trackIndices.guitar.size() > 0)
						{
							m_audio.guitar.volume[0] = reader.extract<float>();
							if (m_trackIndices.guitar.size() > 1)
								m_audio.guitar.volume[1] = reader.extract<float>();
						}

						if (m_trackIndices.vocals.size() > 0)
						{
							m_audio.vocals.volume[0] = reader.extract<float>();
							if (m_trackIndices.vocals.size() > 1)
								m_audio.vocals.volume[1] = reader.extract<float>();
						}

						if (!reader.extract(m_audio.tracks.volume[0]) || !reader.extract(m_audio.tracks.volume[1]))
							return;

						if (m_trackIndices.crowd.size() > 0)
						{
							m_audio.crowd.volume[0] = reader.extract<float>();
							if (m_trackIndices.crowd.size() > 1)
								m_audio.crowd.volume[1] = reader.extract<float>();
						}
					});
				}
				else if (descriptor == "cores")
				{
					reader.runLoop([this, &reader] {
						if (m_trackIndices.drum.size() > 2)
						{
							m_audio.kick.core[0] = reader.extract<float>();
							if (m_trackIndices.drum.size() > 5)
								m_audio.kick.core[1] = reader.extract<float>();
						}

						if (m_trackIndices.drum.size() > 1)
						{
							m_audio.snare.core[0] = reader.extract<float>();
							if (m_trackIndices.drum.size() > 4)
								m_audio.snare.core[1] = reader.extract<float>();
						}

						if (m_trackIndices.drum.size() > 0)
						{
							m_audio.cymbals.core[0] = reader.extract<float>();
							if (m_trackIndices.drum.size() > 3)
								m_audio.cymbals.core[1] = reader.extract<float>();
						}

						if (m_trackIndices.bass.size() > 0)
						{
							m_audio.bass.core[0] = reader.extract<float>();
							if (m_trackIndices.bass.size() > 1)
								m_audio.bass.core[1] = reader.extract<float>();
						}

						if (m_trackIndices.guitar.size() > 0)
						{
							m_audio.guitar.core[0] = reader.extract<float>();
							if (m_trackIndices.guitar.size() > 1)
								m_audio.guitar.core[1] = reader.extract<float>();
						}

						if (m_trackIndices.vocals.size() > 0)
						{
							m_audio.vocals.core[0] = reader.extract<float>();
							if (m_trackIndices.vocals.size() > 1)
								m_audio.vocals.core[1] = reader.extract<float>();
						}
						if (!reader.extract(m_audio.tracks.core[0]) || !reader.extract(m_audio.tracks.core[1]))
							return;

						if (m_trackIndices.crowd.size() > 0)
						{
							m_audio.crowd.core[0] = reader.extract<float>();
							if (m_trackIndices.crowd.size() > 1)
								m_audio.crowd.core[1] = reader.extract<float>();
						}
					});
				}
				else if (descriptor == "midi_file") m_midiFile = reader.extractText();
			});
		}
		else if (name == "song_scroll_speed") m_scrollSpeed = reader.extract<uint32_t>();
		else if (name == "tuning_offset_cents") m_tuningOffsetCents = reader.extract<int32_t>();
		else if (name == "bank") m_vocalPercBank = reader.extractText();
		else if (name == "anim_tempo")
		{
			std::string_view val = reader.extractText();
			if (val == "kTempoSlow") m_animTempo = 16;
			else if (val == "kTempoMedium") m_animTempo = 32;
			else if (val == "kTempoFast") m_animTempo = 64;
			else m_animTempo = std::strtoul(val.data(), nullptr, 0);
		}
		else if (name == "preview")
		{
			m_preview[0] = reader.extract<uint32_t>();
			m_preview[1] = reader.extract<uint32_t>();
		}
		else if (name == "rank")
		{
			reader.runLoop([this, &reader] {
				std::string_view trackname = reader.getNameOfNode();
				if (trackname == "drum") m_ranks.drum4_pro = reader.extract<uint16_t>();
				else if (trackname == "guitar") m_ranks.guitar5 = reader.extract<uint16_t>();
				else if (trackname == "bass") m_ranks.bass5 = reader.extract<uint16_t>();
				else if (trackname == "vocals") m_ranks.vocals = reader.extract<uint16_t>();
				else if (trackname == "keys") m_ranks.keys = reader.extract<uint16_t>();
				else if (trackname == "real_guitar") m_ranks.real_guitar = reader.extract<uint16_t>();
				else if (trackname == "real_bass") m_ranks.real_bass = reader.extract<uint16_t>();
				else if (trackname == "real_keys") m_ranks.real_keys = reader.extract<uint16_t>();
				else if (trackname == "band") m_ranks.band = reader.extract<uint16_t>();
			});
		}
		else if (name == "solo") reader.runLoop([this, &reader] { m_soloes.push_back(reader.extractText()); });
		else if (name == "genre") m_genre = reader.extractText();
		else if (name == "decade") m_decade = reader.extractText();
		else if (name == "vocal_gender") m_vocalIsMale = reader.extractText() == "male";
		else if (name == "format") m_format = reader.extract<uint32_t>();
		else if (name == "version") m_version = reader.extract<uint32_t>();
		else if (name == "fake") m_fake = reader.extractText();
		else if (name == "downloaded") m_downloaded = reader.extractText();
		else if (name == "game_origin")
		{
			m_source = reader.extractText();
			if ((m_source == "ugc" || m_source == "ugc_plus"))
			{
				if (!m_nodeName.starts_with("UGC_"))
					m_source = "customs";
			}
			else if (m_source == "rb1" || m_source == "rb1_dlc" || m_source == "rb1dlc" ||
				m_source == "gdrb" || m_source == "greenday" || m_source == "beatles" ||
				m_source == "tbrb" || m_source == "lego" || m_source == "lrb" ||
				m_source == "rb2" || m_source == "rb3" || m_source == "rb3_dlc" || m_source == "rb3dlc")
			{
				m_charter = "Harmonix";
			}
		}
		else if (name == "song_id") m_songID = reader.extractText();
		else if (name == "rating") m_rating = reader.extract<uint32_t>();
		else if (name == "short_version") m_shortVersion = reader.extract<uint32_t>();
		else if (name == "album_art") m_hasAlbumArt = reader.extract<bool>();
		else if (name == "year_released") m_year_released = reader.extract<uint32_t>();
		else if (name == "year_recorded") m_year_recorded = reader.extract<uint32_t>();
		else if (name == "album_name") m_album = reader.extractText();
		else if (name == "album_track_number") m_albumTrack = reader.extract<uint32_t>();
		else if (name == "pack_name") m_packname = reader.extractText();
		else if (name == "base_points") m_basePoints = reader.extract<uint32_t>();
		else if (name == "band_fail_cue") m_bandFailCue = reader.extractText();
		else if (name == "drum_bank") m_drumBank = reader.extractText();
		else if (name == "song_length") m_length = reader.extract<uint32_t>();
		else if (name == "sub_genre") m_subgenre = reader.extractText();
		else if (name == "author") m_charter = reader.extractText();
		else if (name == "guide_pitch_volume") m_guidePitchVolume = reader.extract<float>();
		else if (name == "encoding") m_encoding = reader.extractText();
		else if (name == "vocal_tonic_note") m_vocalTonic = reader.extract<uint32_t>();
		else if (name == "song_tonality") m_tonality = reader.extract<bool>();
		else if (name == "real_guitar_tuning")
		{
			reader.runLoop([this, &reader] {
				for (int16_t& tuning : m_realGuitarTuning)
					tuning = reader.extract<int16_t>();
			});
		}
		else if (name == "real_bass_tuning")
		{
			reader.runLoop([this, &reader] {
				for (int16_t& tuning : m_realBassTuning)
					tuning = reader.extract<int16_t>();
			});
		}
		else if (name == "video_venues") reader.runLoop([this, &reader] { m_videoVenues.push_back(reader.extractText()); });
		else m_others.push_back({ name, reader.extractText() });
	});
}

const DTA::DTARanks& DTA::DTAFileNode::getRanks() const noexcept { return m_ranks; }
