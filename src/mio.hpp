#pragma once
#include <string>
#include "filesystem.hpp"
#include "types.hpp"

struct MIO {
	constexpr static u8 HEADER[16] = { 0x11, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 'D', 'S', 'M', 'I', 'O', '_', 'S', 0x00 };

	struct Record {
		constexpr static int MAX_PHRASES = 24;
		constexpr static int TRACK_COUNT = 4;
		constexpr static int TRACK_LENGTH = 32;
		constexpr static int RHYTHM_SIMULTANEOUS_NOTES = 4;
		constexpr static u8 NO_NOTE = 0xFF;

		struct Track {
			u8 volume;
			u8 panning;
			u8 instrumentSet;
			u8 notes[TRACK_LENGTH];
		};

		struct RhythmTrack {
			u8 volume;
			u8 panning;
			u8 instrumentSet;
			u8 notes[RHYTHM_SIMULTANEOUS_NOTES][TRACK_LENGTH];
		};

		struct Phrase {
			Track tracks[TRACK_COUNT];
			RhythmTrack rhythmTrack;
		};

		static u16 tempoToBPM(u8 tempo) {
			return 60 + tempo * 10;
		}

		u16 bpm() const {
			return tempoToBPM(tempo);
		}

		bool swing;
		u8 tempo;
		u8 endPhrase;

		Phrase phrases[MAX_PHRASES];
	};

	void load(const fs::path& path);

	std::string formatSerial() const;

	char name[24+1];
	char brand[18+1];
	char creator[18+1];
	char description[72+1];

	char serial1[4+1];
	u32 serial2;
	u16 serial3;

	Record recordData;
};
