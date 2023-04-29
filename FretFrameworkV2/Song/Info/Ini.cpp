#include "Ini.h"

std::vector<Modifiers::Modifier> Ini::ReadSongIniFile(const std::filesystem::path& path)
{
	static const std::pair<std::string_view, IniFileReader::ModifierOutline> SONG_INI_MODIFIERS =
	{
		"[song]",
		{
			{ "album",                                { "album",   ModifierNode::STRING } },
			{ "album_track",                          { "album_track",   ModifierNode::UINT16 } },
			{ "artist",                               { "artist",   ModifierNode::STRING } },

			{ "background",                           { "background",   ModifierNode::STRING_NOCASE } },
			{ "banner_link_a",                        { "banner_link_a",   ModifierNode::STRING_NOCASE } },
			{ "banner_link_b",                        { "banner_link_b",   ModifierNode::STRING_NOCASE } },
			{ "bass_type",                            { "bass_type",   ModifierNode::UINT32 } },
			{ "boss_battle",                          { "boss_battle",   ModifierNode::BOOL } },

			{ "cassettecolor",                        { "cassettecolor",   ModifierNode::UINT32 } },
			{ "charter",                              { "charter",   ModifierNode::STRING } },
			{ "count",                                { "count",   ModifierNode::UINT32 } },
			{ "cover",                                { "cover",   ModifierNode::STRING_NOCASE } },

			{ "dance_type",                           { "dance_type",   ModifierNode::UINT32 } },
			{ "delay",                                { "delay",   ModifierNode::FLOAT} },
			{ "diff_band",                            { "diff_band",   ModifierNode::INT32 } },
			{ "diff_bass",                            { "diff_bass",   ModifierNode::INT32 } },
			{ "diff_bass_real",                       { "diff_bass_real",   ModifierNode::INT32 } },
			{ "diff_bass_real_22",                    { "diff_bass_real_22",   ModifierNode::INT32 } },
			{ "diff_bassghl",                         { "diff_bassghl",   ModifierNode::INT32 } },
			{ "diff_dance",                           { "diff_dance",   ModifierNode::INT32 } },
			{ "diff_drums",                           { "diff_drums",   ModifierNode::INT32 } },
			{ "diff_drums_real",                      { "diff_drums_real",   ModifierNode::INT32 } },
			{ "diff_drums_real_ps",                   { "diff_drums_real_ps",   ModifierNode::INT32 } },
			{ "diff_guitar",                          { "diff_guitar",   ModifierNode::INT32 } },
			{ "diff_guitar_coop",                     { "diff_guitar_coop",   ModifierNode::INT32 } },
			{ "diff_guitar_real",                     { "diff_guitar_real",   ModifierNode::INT32 } },
			{ "diff_guitar_real_22",                  { "diff_guitar_real_22",   ModifierNode::INT32 } },
			{ "diff_guitarghl",                       { "diff_guitarghl",   ModifierNode::INT32 } },
			{ "diff_keys",                            { "diff_keys",   ModifierNode::INT32 } },
			{ "diff_keys_real",                       { "diff_keys_real",   ModifierNode::INT32 } },
			{ "diff_keys_real_ps",                    { "diff_keys_real_ps",   ModifierNode::INT32 } },
			{ "diff_rhythm",                          { "diff_rhythm",   ModifierNode::INT32 } },
			{ "diff_vocals",                          { "diff_vocals",   ModifierNode::INT32 } },
			{ "diff_vocals_harm",                     { "diff_vocals_harm",   ModifierNode::INT32 } },
			{ "drum_fallback_blue",                   { "drum_fallback_blue",   ModifierNode::BOOL } },

			{ "early_hit_window_size",                { "early_hit_window_size",   ModifierNode::STRING_NOCASE } },
			{ "eighthnote_hopo",                      { "eighthnote_hopo",   ModifierNode::UINT32 } },
			{ "end_events",                           { "end_events",   ModifierNode::BOOL } },
			{ "eof_midi_import_drum_accent_velocity", { "eof_midi_import_drum_accent_velocity",  ModifierNode::UINT16 } },
			{ "eof_midi_import_drum_ghost_velocity",  { "eof_midi_import_drum_ghost_velocity",  ModifierNode::UINT16 } },

			{ "five_lane_drums",                      { "five_lane_drums",   ModifierNode::BOOL } },
			{ "frets",                                { "charter",   ModifierNode::STRING } },

			{ "genre",                                { "genre",   ModifierNode::STRING } },
			{ "guitar_type",                          { "guitar_type",   ModifierNode::UINT32 } },

			{ "hopo_frequency",                       { "hopo_frequency",   ModifierNode::UINT32 } },

			{ "icon",                                 { "icon",   ModifierNode::STRING_NOCASE } },

			{ "keys_type",                            { "keys_type",   ModifierNode::UINT32 } },
			{ "kit_type",                             { "kit_type",   ModifierNode::UINT32 } },

			{ "link_name_a",                          { "link_name_a",   ModifierNode::STRING_NOCASE } },
			{ "link_name_b",                          { "link_name_b",   ModifierNode::STRING_NOCASE } },
			{ "loading_phrase",                       { "loading_phrase",   ModifierNode::STRING_NOCASE } },
			{ "lyrics",                               { "lyrics",   ModifierNode::BOOL } },

			{ "modchart",                             { "modchart",   ModifierNode::BOOL } },
			{ "multiplier_note",                      { "star_power_note",   ModifierNode::UINT16 } },

			{ "name",                                 { "name",   ModifierNode::STRING } },

			{ "playlist",                             { "playlist",   ModifierNode::STRING } },
			{ "playlist_track",                       { "playlist_track",   ModifierNode::UINT16 } },
			{ "preview",                              { "preview",   ModifierNode::FLOATARRAY } },
			{ "preview_end_time",                     { "preview_end_time",   ModifierNode::FLOAT} },
			{ "preview_start_time",                   { "preview_start_time",   ModifierNode::FLOAT} },

			{ "pro_drum",                             { "pro_drums",   ModifierNode::BOOL } },
			{ "pro_drums",                            { "pro_drums",   ModifierNode::BOOL } },

			{ "rating",                               { "rating",   ModifierNode::UINT32 } },
			{ "real_bass_22_tuning",                  { "real_bass_22_tuning",   ModifierNode::UINT32 } },
			{ "real_bass_tuning",                     { "real_bass_tuning",   ModifierNode::UINT32 } },
			{ "real_guitar_22_tuning",                { "real_guitar_22_tuning",   ModifierNode::UINT32 } },
			{ "real_guitar_tuning",                   { "real_guitar_tuning",   ModifierNode::UINT32 } },
			{ "real_keys_lane_count_left",            { "real_keys_lane_count_left",   ModifierNode::UINT32 } },
			{ "real_keys_lane_count_right",           { "real_keys_lane_count_right",   ModifierNode::UINT32 } },

			{ "scores",                               { "scores",   ModifierNode::STRING_NOCASE } },
			{ "scores_ext",                           { "scores_ext",   ModifierNode::STRING_NOCASE } },
			{ "song_length",                          { "song_length",   ModifierNode::UINT32 } },
			{ "star_power_note",                      { "star_power_note",   ModifierNode::UINT16 } },
			{ "sub_genre",                            { "sub_genre",   ModifierNode::STRING } },
			{ "sub_playlist",                         { "sub_playlist",   ModifierNode::STRING } },
			{ "sustain_cutoff_threshold",             { "sustain_cutoff_threshold",   ModifierNode::UINT32 } },
			{ "sysex_high_hat_ctrl",                  { "sysex_high_hat_ctrl",   ModifierNode::BOOL } },
			{ "sysex_open_bass",                      { "sysex_open_bass",   ModifierNode::BOOL } },
			{ "sysex_pro_slide",                      { "sysex_pro_slide",   ModifierNode::BOOL } },
			{ "sysex_rimshot",                        { "sysex_rimshot",   ModifierNode::BOOL } },
			{ "sysex_slider",                         { "sysex_slider",   ModifierNode::BOOL } },

			{ "tags",                                 { "tags",   ModifierNode::STRING_NOCASE } },
			{ "track",                                { "album_track",   ModifierNode::UINT16 } },
			{ "tutorial",                             { "tutorial",   ModifierNode::BOOL } },

			{ "unlock_completed",                     { "unlock_completed",   ModifierNode::UINT32 } },
			{ "unlock_id",                            { "unlock_id",   ModifierNode::STRING_NOCASE } },
			{ "unlock_require",                       { "unlock_require",   ModifierNode::STRING_NOCASE } },
			{ "unlock_text",                          { "unlock_text",   ModifierNode::STRING_NOCASE } },

			{ "version",                              { "version",   ModifierNode::UINT32 } },
			{ "video",                                { "video",   ModifierNode::STRING_NOCASE } },
			{ "video_end_time",                       { "video_end_time",   ModifierNode::FLOAT} },
			{ "video_loop",                           { "video_loop",   ModifierNode::BOOL } },
			{ "video_start_time",                     { "video_start_time",   ModifierNode::FLOAT} },
			{ "vocal_gender",                         { "vocal_gender",   ModifierNode::UINT32 } },

			{ "year",                                 { "year",   ModifierNode::STRING } },
		}
	};

	auto modifiers = ReadIniFile(path, { SONG_INI_MODIFIERS });
	if (modifiers.isEmpty())
		return {};
	return modifiers.back();
}
