#pragma once
#include <optional>
#include <string>
#include <vector>
#include "filesystem.hpp"
#include "types.hpp"

struct IT {
	constexpr static u8 MAGIC[4] = { 'I', 'M', 'P', 'M' };
	constexpr static u16 TRACKER_VERSION = 0xBEEF;
	constexpr static u16 COMPATIBLE_TRACKER_VERSION = 0x0214;
	constexpr static u8 RESERVED = 0;
	constexpr static u8 UNUSED = 0;

	/*
	 * Wasteful to have small arbitrary limits like these when their sizes are typically stored in u16s...
	 * Were they planning for future extension?
	 */
	constexpr static int MAX_INSTRUMENTS = 99;
	constexpr static int MAX_SAMPLES = 99;
	constexpr static int MAX_CHANNELS = 64;

	constexpr static int MIN_ROWS = 32;
	constexpr static int MAX_ROWS = 200;

	constexpr static int MAX_MESSAGE_LENGTH = 8000;

	enum Flags : u16 {
		ITMF_STEREO                 = 1 << 0, // Mono otherwise
		ITMF_VOL0_MIX_OPTIMIZATIONS = 1 << 1, // Redundant
		ITMF_USE_INSTRUMENTS        = 1 << 2, // Samples otherwise
		ITMF_LINEAR_SLIDES          = 1 << 3, // Amiga slides otherwise
		ITMF_OLD_EFFECTS            = 1 << 4, // IT effects otherwise
		ITMF_COMPAT_GXX             = 1 << 5,
		ITMF_MIDI_PITCH_CONTROLLER  = 1 << 6,
		ITMF_EMBEDDED_MIDI_CONFIG   = 1 << 7,
		ITMF_EXTENDED_FILTER_RANGE  = 1 << 8  // Is this OpenMPT specific?
	};

	enum SpecialFlags : u16 {
		ITMSF_SONG_MESSAGE       = 1 << 0,
		ITMSF_EDIT_HISTORY       = 1 << 1,
		ITMSF_PATTERN_HIGHLIGHTS = 1 << 2,
		ITMSF_MIDI_CONFIGURATION = 1 << 3
	};

	enum OrderMarkers : u8 {
		ITMOM_SKIP_TO_NEXT = 254,
		ITMOM_END_OF_SONG = 255
	};

	enum PanValues : u8 {
		ITPV_LEFT     = 0,
		ITPV_MIDDLE   = 32,
		ITPV_RIGHT    = 64,
		ITPV_SURROUND = 100,
		ITPV_DISABLED = 128
	};

	enum EnvelopeFlags : u8 {
		ITEF_ENVELOPE        = 1 << 0,
		ITEF_LOOP            = 1 << 1,
		ITEF_SUSTAIN_LOOP    = 1 << 2,
		ITEF_FILTER_ENVELOPE = 1 << 7 // Use pitch envelope as filter envelope instead
	};

	enum SampleFlags : u8 {
		ITSF_SAMPLE_HEADER          = 1 << 0, // ??? idk what this is
		ITSF_SAMPLE_16BIT           = 1 << 1, // 8 bit otherwise
		ITSF_STEREO                 = 1 << 2, // Mono otherwise
		ITSF_COMPRESSED_SAMPLES     = 1 << 3,
		ITSF_LOOP                   = 1 << 4,
		ITSF_SUSTAIN_LOOP           = 1 << 5,
		ITSF_PING_PONG_LOOP         = 1 << 6, // Forwards loop otherwise
		ITSF_PING_PONG_SUSTAIN_LOOP = 1 << 7  // Forwards sustain loop otherwise
	};

	// Other bits are just for internal use only, apparently
	enum SampleConvertFlags : u8 {
		ITSCF_SIGNED = 1 << 0 // Unsigned otherwise
	};

	enum NoteValues : u8 {
		ITNV_NOTE_CUT = 254,
		ITNV_NOTE_OFF = 255
	};

	enum VolPanRanges : u8 {
		ITVPR_VOL_START              = 0,
		ITVPR_VOL_END                = 64,
		ITVPR_FINE_VOL_UP_START      = 65,
		ITVPR_FINE_VOL_UP_END        = 74,
		ITVPR_FINE_VOL_DOWN_START    = 75,
		ITVPR_FINE_VOL_DOWN_END      = 84,
		ITVPR_VOL_SLIDE_UP_START     = 85,
		ITVPR_VOL_SLIDE_UP_END       = 94,
		ITVPR_VOL_SLIDE_DOWN_START   = 95,
		ITVPR_VOL_SLIDE_DOWN_END     = 104,
		ITVPR_PITCH_SLIDE_DOWN_START = 105,
		ITVPR_PITCH_SLIDE_DOWN_END   = 114,
		ITVPR_PITCH_SLIDE_UP_START   = 115,
		ITVPR_PITCH_SLIDE_UP_END     = 124, // 3 value gap?
		ITVPR_PAN_START              = 128,
		ITVPR_PAN_END                = 192,
		ITVPR_PORTAMENTO_TO_START    = 193,
		ITVPR_PORTAMENTO_TO_END      = 202,
		ITVPR_VIBRATO_START          = 203,
		ITVPR_VIBRATO_END            = 212,
		ITVPR_NULL                   = 255
	};

	enum class NewNoteAction : u8 {
		Cut      = 0,
		Continue = 1,
		Off      = 2,
		Fade     = 3
	};

	enum class DuplicateCheckType : u8 {
		Off        = 0,
		Note       = 1,
		Sample     = 2,
		Instrument = 3
	};

	enum class DuplicateCheckAction : u8 {
		Cut  = 0,
		Off  = 1,
		Fade = 2
	};

	enum class VibratoType : u8 {
		SineWave   = 0,
		RampDown   = 1, // Is this not just a sawtooth?
		SquareWave = 2,
		Random     = 3
	};

	/**
	 * Ugh, probably easier to just use std::optional for note at this point.
	 * Seemingly all values are used up so it's not like I can use 0 or a
	 * special constant as a "NO_NOTE" value or something.
	 */
	struct Note {
		std::optional<u8> note;
		u8 instrument = 0;
		u8 volume     = ITVPR_NULL;
		u8 effect     = 0;
		u8 param      = 0;
	};

	struct Pattern {
		u16 rows;
		Note data[MAX_CHANNELS][MAX_ROWS];
	};

	struct Sample {
		constexpr static u8 MAGIC[4] = { 'I', 'M', 'P', 'S' };

		char dosFilename[12+1]{};

		u8 globalVol  = 64; // 0..64
		SampleFlags flags{0};
		u8 defaultVol = 64;

		char name[25+1]{};

		SampleConvertFlags convertFlags{0};

		u8 defaultPan = 0; // TODO

		u32 length    = 0;
		u32 loopBegin = 0;
		u32 loopEnd   = 0;

		u32 c5Speed = 8363; // Bytes a second. 0..9999999

		u32 sustainLoopBegin = 0;
		u32 sustainLoopEnd   = 0;

		u8 vibratoSpeed = 0; // 0..64
		u8 vibratoDepth = 0; // 0..64
		u8 vibratoRate  = 0; // 0..64
		VibratoType vibratoType{VibratoType::SineWave};

		std::vector<u8> data;
	};

	struct Envelope {
		constexpr static size_t MAX_POINTS = 25;

		struct Point {
			s8 y; // 0..64 for volume, -32..+32 for panning & pitch
			u16 tick; // 0..9999
		};

		EnvelopeFlags flags{0};
		u8 numPoints = 0;
		
		u8 loopBegin = 0;
		u8 loopEnd   = 0;
		u8 sustainLoopBegin = 0;
		u8 sustainLoopEnd   = 0;

		Point points[MAX_POINTS]{};
	};

	struct NoteSamplePair {
		u8 note;
		u8 sample;
	};

	struct Instrument {
		constexpr static u8 MAGIC[4] = { 'I', 'M', 'P', 'I' };

		char dosFilename[12+1]{};

		NewNoteAction nna{ NewNoteAction::Cut };
		DuplicateCheckType dct{ DuplicateCheckType::Off };
		DuplicateCheckAction dca{ DuplicateCheckAction::Cut };

		u16 fadeOut = 0; // 0..128?

		s8 pitchPanSep = 0; // -32..+32
		u8 pitchPanCenter; // 0..119 (C-0 to B-9) [TODO]

		u8 globalVolume = 128; // 0..128
		u8 defaultPan   = 32 | 128; // 0..64 (AND 128 = "don't use")

		u8 randVolVar = 0; // Percentage?
		u8 randPanVar = 0; // ITTECH says "not implemented yet"

		char name[25+1]{};

		u8 initialFilterCutoff; // TODO
		u8 initialFilterResonance;

		u8 midiChannel = 0;
		s8 midiProgram = -1;
		s8 midiBankLSB = -1;
		s8 midiBankMSB = -1;

		NoteSamplePair keyboard[120]{};

		Envelope volEnv;
		Envelope panEnv;
		Envelope pitchEnv;
	};

	struct Channel {
		u8 pan    = 32; // 0..64
		u8 volume = 64; // 0..64
	};

	void save(const fs::path& path) const;

	char name[25+1]{};

	u8 highlightRowsPerBeat    = 4;
	u8 highlightRowsPerMeasure = 16;

	Flags flags{ ITMF_STEREO | ITMF_LINEAR_SLIDES };
	SpecialFlags specialFlags{ ITMSF_SONG_MESSAGE | ITMSF_PATTERN_HIGHLIGHTS };

	u8 globalVolume = 128; // 0..128
	u8 mixVolume    = 48;  // 0..128

	u8 initialSpeed = 6; // Ticks per row
	u8 initialTempo = 125;

	u8 panSeparation       = 128; // 0..128
	u8 midiPitchWheelDepth = 0;

	std::string message;

	Channel channels[MAX_CHANNELS];

	std::vector<u8> orders;
	std::vector<Instrument> instruments;
	std::vector<Sample> samples;
	std::vector<Pattern> patterns;
};
