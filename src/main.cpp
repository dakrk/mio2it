#include <cstdio>
#include <cstring>
#include "it.hpp"
#include "mio.hpp"

static u8 MIOToITPanTable[] = { 0, 64, 128, 192, 255 };

constexpr static u8 letterInAlphabet(char c) {
	return c - ('A' - 1);
}

static void convertMIO(const fs::path& mioPath, const fs::path& itPath) {
	IT it;
	MIO mio;

	mio.load(mioPath);

	MIO::Record& record = mio.recordData;

	printf("Name: %s\n", mio.name);
	printf("Brand: %s\n", mio.brand);
	printf("Creator: %s\n", mio.creator);
	printf("Description: %s\n", mio.description);
	printf("Serial: %s\n", mio.formatSerial().c_str());

	strncpy(it.name, mio.name, std::size(it.name) - 1);
	it.initialTempo = record.bpm();
	it.message = mio.description;

	it.orders.resize(record.endPhrase);
	it.patterns.reserve(MIO::Record::MAX_PHRASES);

	for (int i = 0; i < record.endPhrase; i++) {
		it.orders[i] = i;
	}

	// Set default value to 2 as that is center
	u8 lastTrackPan[MIO::Record::TRACK_COUNT + 1];
	std::fill_n(lastTrackPan, MIO::Record::TRACK_COUNT + 1, 2);

	for (const MIO::Record::Phrase& phrase : record.phrases) {
		IT::Pattern pattern;
		pattern.rows = MIO::Record::TRACK_LENGTH;

		// Write all four normal tracks
		for (int t = 0; t < MIO::Record::TRACK_COUNT; t++) {
			if (phrase.tracks[t].panning != lastTrackPan[t]) {
				pattern.data[t][0].effect = letterInAlphabet('X');
				pattern.data[t][0].param = MIOToITPanTable[phrase.tracks[t].panning];				
			}

			lastTrackPan[t] = phrase.tracks[t].panning;

			for (int n = 0; n < MIO::Record::TRACK_LENGTH; n++) {
				u8 note = phrase.tracks[t].notes[n];
				if (note != MIO::Record::NO_NOTE) {
					pattern.data[t][n].note = note + (7 + (12 * 3));
					pattern.data[t][n].volume = phrase.tracks[t].volume * 16;
				}
			}
		}

		// Write rhythm track
		for (int p = 0; p < MIO::Record::RHYTHM_SIMULTANEOUS_NOTES; p++) {
			if (phrase.rhythmTrack.panning != lastTrackPan[4]) {
				pattern.data[4 + p][0].effect = letterInAlphabet('X');
				pattern.data[4 + p][0].param = MIOToITPanTable[phrase.rhythmTrack.panning];
			}

			for (int n = 0; n < MIO::Record::TRACK_LENGTH; n++) {
				u8 note = phrase.rhythmTrack.notes[p][n];
				if (note != MIO::Record::NO_NOTE) {
					pattern.data[4 + p][n].note = note + (7 + (12 * 3));
					pattern.data[4 + p][n].volume = phrase.rhythmTrack.volume * 16;
				}
			}
		}

		lastTrackPan[4] = phrase.rhythmTrack.panning;

		it.patterns.push_back(pattern);
	}

	it.save(itPath);
}

int main(int argc, char** argv) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <in.mio> <out.it>\n", argv[0]);
		return 1;
	}

	try {
		convertMIO(argv[1], argv[2]);
	} catch (std::runtime_error& err) {
		fprintf(stderr, "An error occurred: %s\n", err.what());
		return 1;
	}

	return 0;
}
