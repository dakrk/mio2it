#pragma author DarkOK
#pragma description Made in Ore (WarioWare DIY) title data (INCOMPLETE)

#pragma endian little
#pragma loop_limit 65536

#define unk padding

import std.array;
import std.io;
import std.mem;
import std.sys;

namespace mio {
    fn calcTempo(u8 tempo) {
        return 60 + tempo * 10;
    };
    
    enum TitleType : u8 {
        Game = 0x00,
        Record = 0x01,
        Comic = 0x02
    };
    
    const auto TRACK_COUNT = 4; // Not including rhythm track
    const auto TRACK_LENGTH = 32;
    const auto MAX_PHRASES = 24;
    const auto RHYTHM_SIMULTANEOUS_NOTES = 4;
    
    struct Header {
        unk[16];
        u32 headerChecksum; // Excluding the checksums at 0x10-0x17.
        u32 dataChecksum;
        
        unk[4];

        char name[25];
        char brand[19];
        char creator[19];
        char description[73];
        
        TitleType type;
        
        // TODO: This contains appearance and stuff
        unk[42];
        
        char serial1[5];
        u32 serial2;
        u16 serial3;
        
        unk[14];
        bool locked;
        
        unk[5];
        u16 timestamp;
        
        unk[16];
    };
    
    struct RecordHeader {
        bool swing;
        u8 tempo [[format("mio::calcTempo")]];
        u8 endPhrase; // Indicated by the flag in-game
        unk[4];
    };

    /**
     * MIO note 0 is equivalent to G-3.
     * Some instruments may have octaves differently, though.
     */
    struct RecordTrack {
        u8 notes[TRACK_LENGTH];
    };
    
    struct RecordRhythmTrack {
        std::Array<std::Array<u8, TRACK_LENGTH>, RHYTHM_SIMULTANEOUS_NOTES> notes;
        // u8 notes[TRACK_LENGTH * RHYTHM_SIMULTANEOUS_NOTES];
    };
    
    // Volume and panning is in the range of 0 to 4.
    struct RecordPhrase {
        RecordTrack tracks[TRACK_COUNT];
        RecordRhythmTrack rhythmTrack;
        
        u8 volume[TRACK_COUNT + 1];
        u8 panning[TRACK_COUNT + 1];
        u8 instruments[TRACK_COUNT + 1];
        unk[5];
    };
    
    struct RecordData {
        RecordHeader header;
        RecordPhrase phrases[MAX_PHRASES];
    };
    
    // TODO: Account for padding at the end of file
    struct MIO {
        Header header;
        
        match (header.type) {
            (TitleType::Record): RecordData data;
            (_): std::warning(std::format("Unsupported title type {}, cannot continue", header.type));
        }
    };

    fn calculateChecksums() {
        u32 newHeaderChecksum;
        u32 newDataChecksum;
    
        u32 i;
        for (i = 0x0, i < 0x100, i += sizeof(u8)) {
            // Skip the checksum region
            if (i >= 0x10 && i < 0x17)
                continue;
                
            u8 d @ i;
            newHeaderChecksum += d & 0xFF;
        }
        
        for (i = 0x100, i < std::mem::size(), i += sizeof(u8)) {     
            u8 d @ i;
            newDataChecksum += d & 0xFF;
        }
        
        std::print("Current header checksum = {0:x} ({0}) ", headerChecksum);
        std::print("Recalculated header checksum = {0:x} ({0})", newHeaderChecksum);
        std::print("");
        std::print("Current data checksum = {0:x} ({0})", dataChecksum);
        std::print("Recalculated data checksum = {0:x} ({0})", newDataChecksum);
        std::print("");
    };
}

mio::MIO mio @ 0x0;