#include "io.hpp"
#include "it.hpp"

enum PatternMaskBits : u8 {
	ITPMB_NOTE            = 1 << 0,
	ITPMB_INSTRUMENT      = 1 << 1,
	ITPMB_VOL_PAN         = 1 << 2,
	ITPMB_COMMAND         = 1 << 3,
	ITPMB_LAST_NOTE       = 1 << 4,
	ITPMB_LAST_INSTRUMENT = 1 << 5,
	ITPMB_LAST_VOL_PAN    = 1 << 6,
	ITPMB_LAST_COMMAND    = 1 << 7
};

static void writeEnvelope(io::FileIO& file, const IT::Envelope& env) {
	file.writeU8(env.flags);
	file.writeU8(env.numPoints);
	file.writeU8(env.loopBegin);
	file.writeU8(env.loopEnd);
	file.writeU8(env.sustainLoopBegin);
	file.writeU8(env.sustainLoopEnd);
	file.writeArrT(env.points); // Hope the members are in the right order
}

void IT::save(const fs::path& path) const {
	io::FileIO file(path, "wb");

	u32 lastPos;
	u32 lastPos2;

	/* ================================ *
	 *      Initial validity tests      *
	 * ================================ */

	if (instruments.size() > MAX_INSTRUMENTS) {
		throw std::runtime_error("IT has more than 99 instruments");
	}

	if (samples.size() > MAX_SAMPLES) {
		throw std::runtime_error("IT has more than 99 samples");
	}

	// Do the equal to because of the null terminator
	if (message.size() >= MAX_MESSAGE_LENGTH) {
		throw std::runtime_error("IT message is longer than 8000 bytes");
	}

	/* ======================= *
	 *      Module header      *
	 * ======================= */

	u32 messageOffset;
	u32 instrumentOffsets;
	u32 sampleOffsets;
	u32 patternOffsets;

	file.writeArrT(MAGIC);
	file.writeArrT(name);

	file.writeU8(highlightRowsPerBeat);
	file.writeU8(highlightRowsPerMeasure);

	file.writeU16LE(orders.size());
	file.writeU16LE(instruments.size());
	file.writeU16LE(samples.size());
	file.writeU16LE(patterns.size());

	file.writeU16LE(TRACKER_VERSION);
	file.writeU16LE(COMPATIBLE_TRACKER_VERSION);

	file.writeU16LE(flags);
	file.writeU16LE(specialFlags);

	file.writeU8(globalVolume);
	file.writeU8(mixVolume);

	file.writeU8(initialSpeed);
	file.writeU8(initialTempo);

	file.writeU8(panSeparation);
	file.writeU8(midiPitchWheelDepth);

	file.writeU16LE(message.size() + 1);
	messageOffset = file.tell();
	file.writeU32LE(0);

	file.writeU32LE(RESERVED);

	// Small enough a type to copy, fits in 16 bits
	for (Channel ch : channels)
		file.writeU8(ch.pan);
	for (Channel ch : channels)
		file.writeU8(ch.volume);

	// Vector only contains byte values, no issues here
	file.writeVec(orders);

	instrumentOffsets = file.tell();
	file.writeN(u32(0), instruments.size());

	sampleOffsets = file.tell();
	file.writeN(u32(0), samples.size());

	patternOffsets = file.tell();
	file.writeN(u32(0), patterns.size());

	// The IO string method should really be writing the null-terminator itself
	lastPos = file.tell();
	file.jump(messageOffset);
	file.writeU32LE(lastPos);
	file.jump(lastPos);
	file.writeStr(message);
	file.writeU8('\0');

	/* ===================== *
	 *      Instruments      *
	 * ===================== */

	for (size_t i = 0; i < instruments.size(); i++) {
		const Instrument& instr = instruments[i];

		lastPos = file.tell();
		file.jump(instrumentOffsets + (i * sizeof(u32)));
		file.writeU32LE(lastPos);
		file.jump(lastPos);

		file.writeArrT(Instrument::MAGIC);
		file.writeArrT(instr.dosFilename);

		file.writeU8(static_cast<u8>(instr.nna));
		file.writeU8(static_cast<u8>(instr.dct));
		file.writeU8(static_cast<u8>(instr.dca));

		file.writeU16LE(instr.fadeOut);

		file.writeS8(instr.pitchPanSep);
		file.writeU8(instr.pitchPanCenter);

		file.writeU8(instr.globalVolume);
		file.writeU8(instr.defaultPan);

		file.writeU8(instr.randVolVar);
		file.writeU8(instr.randPanVar);

		// TrkVers and NoS. Not relevant for us
		file.writeU16LE(UNUSED);
		file.writeU8(UNUSED);
		file.writeU8(RESERVED);

		file.writeArrT(instr.name);

		file.writeU8(instr.initialFilterCutoff);
		file.writeU8(instr.initialFilterResonance);

		file.writeU8(instr.midiChannel);
		file.writeS8(instr.midiProgram);
		file.writeS8(instr.midiBankLSB);
		file.writeS8(instr.midiBankMSB);

		// Sure hope each struct member is in the right order when writing this way
		file.writeArrT(instr.keyboard);

		writeEnvelope(file, instr.volEnv);
		writeEnvelope(file, instr.panEnv);
		writeEnvelope(file, instr.pitchEnv);
	}

	/* ================= *
	 *      Samples      *
	 * ================= */

	for (size_t i = 0; i < samples.size(); i++) {
		const Sample& smpl = samples[i];

		lastPos = file.tell();
		file.jump(sampleOffsets + (i * sizeof(u32)));
		file.writeU32LE(lastPos);
		file.jump(lastPos);

		file.writeArrT(Sample::MAGIC);
		file.writeArrT(smpl.dosFilename);

		file.writeU8(RESERVED);
		file.writeU8(smpl.globalVol);
		file.writeU8(smpl.flags);
		file.writeU8(smpl.defaultVol);

		file.writeArrT(smpl.name);

		file.writeU8(smpl.convertFlags);

		file.writeU8(smpl.defaultPan);

		file.writeU32LE(smpl.length);
		file.writeU32LE(smpl.loopBegin);
		file.writeU32LE(smpl.loopEnd);

		file.writeU32LE(smpl.c5Speed);

		file.writeU32LE(smpl.sustainLoopBegin);
		file.writeU32LE(smpl.sustainLoopEnd);

		/**
		 * I *could* just write the sample data after the header, but might as
		 * well be compliant to what other things do and write it all sequentially.
		 */
		file.writeU32LE(0);

		file.writeU8(smpl.vibratoSpeed);
		file.writeU8(smpl.vibratoDepth);
		file.writeU8(smpl.vibratoRate);
		file.writeU8(static_cast<u8>(smpl.vibratoType));
	}

	/* ================== *
	 *      Patterns      *
	 * ================== */

	for (size_t i = 0; i < patterns.size(); i++) {
		const Pattern& pat = patterns[i];

		if (pat.rows > MAX_ROWS) {
			throw std::runtime_error("IT pattern has more than 200 rows");
		}

		lastPos = file.tell();
		file.jump(patternOffsets + (i * sizeof(u32)));
		file.writeU32LE(lastPos);
		file.jump(lastPos);

		file.writeU16LE(0); // Packed length, not yet known
		file.writeU16LE(pat.rows);
		file.writeU32LE(RESERVED);

		for (u32 r = 0; r < pat.rows; r++) {
			for (u32 c = 0; c < MAX_CHANNELS; c++) {
				Note note = pat.data[c][r];
				u8 mask = 0;

				if (note.note.has_value())     mask |= ITPMB_NOTE;
				if (note.instrument)           mask |= ITPMB_INSTRUMENT;
				if (note.volume != ITVPR_NULL) mask |= ITPMB_VOL_PAN;
				if (note.effect || note.param) mask |= ITPMB_COMMAND;

				// TODO: Implement pattern optimisation

				file.writeU8((c + 1) | 0x80);
				file.writeU8(mask);

				if (mask & ITPMB_NOTE)       file.writeU8(note.note.value());
				if (mask & ITPMB_INSTRUMENT) file.writeU8(note.instrument);
				if (mask & ITPMB_VOL_PAN)    file.writeU8(note.volume);
				if (mask & ITPMB_COMMAND) {
					file.writeU8(note.effect);
					file.writeU8(note.param);
				}
			}
			file.writeU8(0);
		}

		lastPos2 = file.tell();
		file.jump(lastPos);
		file.writeU16LE(lastPos2 - (lastPos + 8));
		file.jump(lastPos2);
	}
}
