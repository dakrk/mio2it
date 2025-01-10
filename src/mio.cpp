#include <cstring>
#include "io.hpp"
#include "mio.hpp"

void MIO::load(const fs::path& path) {
	io::FileIO file(path, "rb");

	u8 header[sizeof(HEADER)];
	u8 titleType;

	/* ==================== *
	 *      MIO header      *
	 * ==================== */

	file.readArrT(header);
	if (memcmp(header, HEADER, sizeof(HEADER))) {
		throw std::runtime_error("MIO has invalid header");
	}

	// TODO: Check checksums
	file.forward(12); // checksums + unknown

	file.readStrT(this->name);
	file.readStrT(this->brand);
	file.readStrT(this->creator);
	file.readStrT(this->description);

	file.readU8(&titleType);
	if (titleType != 1) {
		throw std::runtime_error("MIO type is not record");
	}

	file.forward(42); // appearance + unknown

	file.readStrT(this->serial1);
	file.readU32LE(&this->serial2);
	file.readU16LE(&this->serial3);

	// TODO: Get timestamp
	file.forward(38); // locked + timestamp + unknown

	/* ======================= *
	 *      Record header      *
	 * ======================= */

	file.readBool(&this->recordData.swing);
	file.readU8(&this->recordData.tempo);
	file.readU8(&this->recordData.endPhrase);
	file.forward(4); // unknown

	/* ===================== *
	 *      Record data      *
	 * ===================== */

	for (size_t p = 0; p < Record::MAX_PHRASES; p++) {
		Record::Phrase& curPhrase = this->recordData.phrases[p];

		for (size_t t = 0; t < Record::TRACK_COUNT; t++)
			file.readArrT(curPhrase.tracks[t].notes);
		file.readArrT(curPhrase.rhythmTrack.notes);

		for (size_t t = 0; t < Record::TRACK_COUNT; t++)
			file.readU8(&curPhrase.tracks[t].volume);
		file.readU8(&curPhrase.rhythmTrack.volume);

		for (size_t t = 0; t < Record::TRACK_COUNT; t++)
			file.readU8(&curPhrase.tracks[t].panning);
		file.readU8(&curPhrase.rhythmTrack.panning);

		for (size_t t = 0; t < Record::TRACK_COUNT; t++)
			file.readU8(&curPhrase.tracks[t].instrumentSet);
		file.readU8(&curPhrase.rhythmTrack.instrumentSet);

		file.forward(5); // unknown
	}
}

// TODO: See how unusually large serial numbers behave in-game
std::string MIO::formatSerial() const {
	char buf[14];
	int ret = snprintf(buf, 14, "%s-%04u-%03u", serial1, serial2 + 1u, serial3);
	return (ret == 13) ? buf : "erro-0000-000";
}
