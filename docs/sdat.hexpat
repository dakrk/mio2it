#pragma author DarkOK
#pragma description Nintendo DS Nitro Composer file formats (SDAT, SBNK, SWAV only)

#pragma pattern_limit 524288
#pragma endian little

import std.io;
import std.string;
import std.sys;

// Information copied from gota7's Nitro Studio 2 documentation, thank you
namespace nitrocomposer {
    enum SoundEncoding : u8 {
        PCMS8    = 0,
        PCMS16   = 1,
        IMAADPCM = 2
    };

    /**
     * REALLY need to see about if I can make visualisation of this better.
     * It's hard to read and traverse through structures that nest these.
     */
    struct RelPtrBase<DT, PT, auto Base> {
        PT relPos;
        if (!relPos) {
            padding[sizeof(PT)];
        } else {
            DT data @ addressof(Base) + relPos;
        }
    };

    using RelPtr16<DT, auto Base> = RelPtrBase<DT, u16, Base>;
    using RelPtr32<DT, auto Base> = RelPtrBase<DT, u32, Base>;

    struct Table<T> {
        u32 length;
        T entries[length];
        // align to 0x4?
    };

    struct RelPtrTable<T, auto Base> {
        u32 length;
        RelPtr32<T, Base> entries[length];
        // align to 0x4?
    };

    using StringTable<auto Base> = RelPtrTable<std::string::NullString, Base>;

    // Should always be 1.0?
    struct Version {
        u8 minor;
        u8 major;
    };

    struct Block {
        char fourCC[4];
        u32 blockSize;
        // align to 0x4?
    };

    struct FileHeader {
        char fourCC[4];
        u16 byteOrder;
        Version version;
        u32 fileSize;
        u16 headerSize;
        u16 numBlocks;

        // Not sure how else I'd approach this in ImHex
        if (byteOrder != 0xFEFF) {
            std::error("File byte order is different than expected.");
        }
    };

    namespace sseq {

    }

    namespace ssar {

    }

    namespace sbnk {
        fn countRegions(u32 regionsStartAddr) {
            u8 num = 0;
            for (u8 i = 0, i < 8, i += 1) {
                u8 val @ regionsStartAddr + i;
                if (val) {
                    num += 1;
                }
            }
            return num;
        };

        enum InstrumentType : u8 {
            Null       = 0, // "no note info"
            PCM        = 1,
            PSG        = 2,
            WhiteNoise = 3,
            DirectPCM  = 4, // "unused"
            Null2      = 5, // "has note info but 0'd out"
            DrumSet    = 16,
            KeySplit   = 17
        };

        struct NoteInfo {
            u16 waveID; // Duty Cycle Type for PSG
            u16 waveArchiveID;
            u8 baseNote;
            u8 attack;
            u8 decay;
            u8 sustain;
            u8 release;
            u8 pan;
        };

        struct ContainedDirectInstrument {
            InstrumentType type;
            padding[1];
            NoteInfo note;
        };

        struct KeySplitInstrument {
            u8 regions[8];

            u8 numRegions = nitrocomposer::sbnk::countRegions(addressof(regions));
            ContainedDirectInstrument instruments[numRegions];
        };

        struct DrumSetInstrument {
            u8 lowerKey;
            u8 upperKey;
            ContainedDirectInstrument keys[upperKey - lowerKey + 1];
        };

        struct DirectInstrument {
            NoteInfo note;
        };

        struct InstrumentEntry<auto Base> {
            InstrumentType type;
            u16 dataOffset;
            padding[1];

            u32 base = addressof(Base);
            match (type) {
                (InstrumentType::PCM ... InstrumentType::Null2): {
                    DirectInstrument data @ base + dataOffset;
                }

                (InstrumentType::DrumSet): {
                    DrumSetInstrument data @ base + dataOffset;
                }

                (InstrumentType::KeySplit): {
                    KeySplitInstrument data @ base + dataOffset;
                }
            }
        };

        /**
         * Yet another victim of the dumb broken ImHex template nesting...
         * Shouldn't need another Table struct just for this.
         */
        struct InstrumentTable<auto Base> {
            u32 length;
            InstrumentEntry<Base> entries[length];
        };

        struct DataBlock : Block {
            padding[32];
            InstrumentTable<parent> instruments;
        };

        struct SBNKFile {
            FileHeader header;
            DataBlock data;
        };
    }

    namespace swav {
        /**
         * "If audio does not loop, loop start/end is 1 for ADPCM, 0 otherwise."
         * I'm not sure whether to trust that for loop end though with SWAR.
         * How else am I meant to determine audio size, instead of just calculating
         * difference between current offset and next (which is hacky and not all
         * likely Nintendo actually did this).
         */
        struct DataHeader {
            SoundEncoding encoding;
            bool loop;
            u16 sampleRate;
            u16 clockTime; // "(33513982 / 2) / SampleRate"
            u16 loopStart;
            u32 loopEnd;
        };

        struct Data {
            DataHeader header;
            u32 numSamples = header.loopStart + header.loopEnd;
            match (header.encoding) {
                (SoundEncoding::PCMS8): {
                    s8 audio[numSamples];
                }

                (SoundEncoding::PCMS16): {
                    s16 audio[numSamples];
                }

                (SoundEncoding::IMAADPCM): {
                    u8 audio[numSamples * 4];
                }
            }
        };

        struct DataBlock : Block {
            Data data [[inline]];
        };

        struct SWAVFile {
            FileHeader header;
            DataBlock data;
        };
    }

    namespace swar {
        struct DataBlock : Block {
            padding[32];
            RelPtrTable<swav::Data, parent> waves;
        };

        struct SWARFile {
            FileHeader header;
            DataBlock data;
        };
    }

    namespace strm {

    }

    struct AutoFile {
        char fourCC[4] [[no_unique_address, hidden]];
        match (fourCC) {
            ("SBNK"): {
                sbnk::SBNKFile data;
            }

            ("SWAR"): {
                swar::SWARFile data;
            }

            (_): {
                u8 data[parent.size];
            }
        }
    };

    namespace sdat {
        enum GroupEntryType : u8 {
            Sequence        = 0,
            Bank            = 1,
            WaveArchive     = 2,
            SequenceArchive = 3
        };

        struct SequenceArchiveSymbolEntry<auto Base> {
            RelPtr32<std::string::NullString, Base> archiveName;
            RelPtr32<StringTable<Base>, Base> sequenceNames;
        };

        /**
         * You know, this is another time where I can't just use the normal Table
         * struct as templates in ImHex are seemingly broken, where trying to nest
         * them (seemingly) thinks you're trying to bitshift something just because
         * it ends with a `>>` and it errors about looking for a value after that.
         */
        struct SequenceArchiveSymbolTable<auto Base> {
            u32 length;
            SequenceArchiveSymbolEntry<Base> entries[length];
            // align to 0x4?
        };

        struct SymbolBlock : Block {
            RelPtr32<StringTable<parent>, this> sequence;
            RelPtr32<SequenceArchiveSymbolTable<parent>, this> sequenceArchive;
            RelPtr32<StringTable<parent>, this> bank;
            RelPtr32<StringTable<parent>, this> waveArchive;
            RelPtr32<StringTable<parent>, this> player;
            RelPtr32<StringTable<parent>, this> group;
            RelPtr32<StringTable<parent>, this> streamPlayer;
            RelPtr32<StringTable<parent>, this> stream;
            padding[24];
        };

        struct SequenceInfo {
            u32 fileID;
            u16 bankID;
            u8 volume;
            u8 channelPriority;
            u8 playerPriority;
            u8 playerID;
            padding[2];
        };

        struct SequenceArchiveInfo {
            u32 fileID;
        };

        struct BankInfo {
            u32 fileID;
            s16 waveArchive[4];
        };

        struct WaveArchiveInfo {
            u24 fileID;
            bool loadIndividually;
        };

        struct PlayerInfo {
            u16 maxSequences;
            u16 channelAllocMask; // "0 allocates all channels automatically"
            u32 heapSize;
        };

        struct GroupEntry {
            GroupEntryType type;
            u8 loadFlags; // Bit 1 is sequence, 2 is bank, etc
            padding[2];
            u32 entryID;
        };

        struct GroupInfo {
            Table<GroupEntry> groups;
        };

        struct StreamPlayerInfo {
            u8 channels;
            u8 channel1;
            u8 channel2;
            padding[21];
        };

        struct StreamInfo {
            u24 fileID;
            bool convertToStereo;
            u8 volume;
            u8 priority;
            u8 streamPlayerID;
            padding[5];
        };

        struct InfoBlock : Block {
            RelPtr32<RelPtrTable<SequenceInfo,        parent>, this> sequence;
            RelPtr32<RelPtrTable<SequenceArchiveInfo, parent>, this> sequenceArchive;
            RelPtr32<RelPtrTable<BankInfo,            parent>, this> bank;
            RelPtr32<RelPtrTable<WaveArchiveInfo,     parent>, this> waveArchive;
            RelPtr32<RelPtrTable<PlayerInfo,          parent>, this> player;
            RelPtr32<RelPtrTable<GroupInfo,           parent>, this> group;
            RelPtr32<RelPtrTable<StreamPlayerInfo,    parent>, this> streamPlayer;
            RelPtr32<RelPtrTable<StreamInfo,          parent>, this> stream;
            padding[24];
        };

        struct FileEntry {
            u32 offset;
            u32 size;
            padding[8];
            AutoFile data @ offset [[inline]];
        };

        struct FATBlock : Block {
            Table<FileEntry> files;
        };

        struct FileBlock : Block {
            u32 numFiles;
        };

        // Not sure what I'd do with all these sizes...
        struct SDATFileHeader : FileHeader {
            u32 symbolBlockOffset;
            u32 symbolBlockSize;
            u32 infoBlockOffset;
            u32 infoBlockSize;
            u32 fatBlockOffset;
            u32 fatBlockSize;
            u32 fileBlockOffset;
            u32 fileBlockSize;
        };

        struct SDATFile {
            SDATFileHeader header;

            if (header.symbolBlockOffset)
                SymbolBlock symbol @ header.symbolBlockOffset;
            if (header.infoBlockOffset)
                InfoBlock info @ header.infoBlockOffset;
            if (header.fatBlockOffset)
                FATBlock fat @ header.fatBlockOffset;
            if (header.fileBlockOffset)
                FileBlock file @ header.fileBlockOffset;
        };
    }
}

nitrocomposer::sdat::SDATFile sdat @ 0x0;
