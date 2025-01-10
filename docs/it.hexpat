#pragma author DarkOK
#pragma description Impulse Tracker module (except patterns)

#pragma endian little

/**
 * Haven't tested all this, so I'm not sure if it's all 100% right.
 * Wrote this as I read ITTECH.TXT.
 */
namespace it {
    const auto MAX_CHANNELS = 64;
    const auto MAX_ENVELOPE_POINTS = 25;

    bitfield ModuleFlags {
        stereo : 1;               // Mono otherwise
        vol0MixOptimizations : 1;
        useInstruments : 1;       // Samples otherwise
        linearSlides : 1;         // Amiga slides otherwise
        oldEffects : 1;           // IT effects otherwise
        compatGXX : 1;
        midiPitchController : 1;
        embeddedMIDIConfig : 1;
        padding : 8;
    };
    
    bitfield ModuleSpecialFlags {
        songMessage : 1;
        editHistory : 1;
        patternHighlights : 1;
        midiConfiguration : 1;
        padding : 8;
    };
    
    bitfield EnvelopeFlags {
        on : 1;
        loop : 1;
        susLoop : 1;
        padding : 4;
        filterEnvelope : 1; // "Use pitch envelope as filter envelope instead"
    };
    
    bitfield SampleFlags {
        sampleHeader : 1;
        bitDepth16Bit : 1;     // 8 bit otherwise (can't begin name with number :P)
        stereo : 1;            // Mono otherwise
        compressedSamples : 1;
        loop : 1;
        susLoop : 1;
        pingPongLoop : 1;      // Forwards loop otherwise
        pingPongSusLoop : 1;   // Forwards sustain loop otherwise
    };    
    
    enum NewNoteAction : u8 {
        Cut = 0,
        Continue = 1,
        Off = 2,
        Fade = 3
    };
    
    enum DuplicateCheckType : u8 {
        Off = 0,
        Note = 1,
        Sample = 2,
        Instrument = 3
    };
    
    enum DuplicateCheckAction : u8 {
        Cut = 0,
        Off = 1,
        Fade = 2
    };
    
    enum VibratoType : u8 {
        SineWave = 0,
        RampDown = 1,
        SquareWave = 2,
        Random = 3
    };
    
    struct Ptr<T> {
        T* data : u32 [[inline]];
    };
    
    struct Pattern {
        u16 length;
        u16 rows;
    };
    
    struct Sample {
        char fourCC[4];
        char dosFileName[13];
        
        u8 globalVolume;
        SampleFlags flags;
        u8 defaultVolume;
        
        u32 length;
        u32 loopBegin;
        u32 loopEnd;
        
        u32 c5Speed;
        
        u32 susLoopBegin;
        u32 susLoopEnd;
        
        u32 samplePtr;
        
        u8 vibratoSpeed;
        u8 vibratoDepth;
        u8 vibratoRate;
        VibratoType vibratoType;
    };
    
    struct EnvelopePoint {
        s8 y;
        u16 tick;
    };
    
    struct Envelope {
        EnvelopeFlags flags;
        u8 numPoints;
        
        u8 loopBegin;
        u8 loopEnd;
        u8 susLoopBegin;
        u8 susLoopEnd;
        
        EnvelopePoint points[MAX_ENVELOPE_POINTS];
    };
    
    struct NoteSamplePair {
        u8 note;
        u8 sample;
    };
    
    struct Instrument {
        char fourCC[4];
        char dosFileName[13];
        
        NewNoteAction nna;
        DuplicateCheckType dct;
        DuplicateCheckAction dca;
        
        u16 fadeOut;
        
        s8 pitchPanSeparation;
        u8 pitchPanCenter;
        
        u8 globalVolume;
        u8 defaultPan;
        
        u8 randVolVar;
        u8 randPanVar;
        
        // Only relevant for ITI files
        u16 trkVers;
        u8 numSamples;
        
        padding[1];
        
        char name[26];
        
        u8 initialFilterCutoff;
        u8 initialFilterResonance;
        
        u8 midiChannel;
        u8 midiProgram;
        u16 midiBank; // should be split up into two u8s
        
        NoteSamplePair keyboard[120];
        
        Envelope volEnv;
        Envelope panEnv;
        Envelope pitchEnv;
    };

    struct Module {
        char fourCC[4];
        char name[26];

        u8 highlightRowsPerBeat;
        u8 highlightRowsPerMeasure;
        
        u16 numOrders;
        u16 numInstruments;
        u16 numSamples;
        u16 numPatterns;
        
        u16 cwt;
        u16 cmwt;
        
        ModuleFlags flags;
        ModuleSpecialFlags specialFlags;
        
        u8 globalVolume;
        u8 mixVolume;
        
        u8 initialSpeed;
        u8 initialTempo;
        
        u8 panSeparation;
        u8 pitchWheelDepth;
        
        u16 messageLength;
        u32 messageOffset;
        char message[messageLength] @ messageOffset;
        
        u32 reserved;
        
        u8 channelPan[MAX_CHANNELS];
        u8 channelVol[MAX_CHANNELS];
        
        u8 orders[numOrders];
        
        if (cmwt < 0x200) {
            // not implemented this yet
        } else {
            Ptr<Instrument> instruments[numInstruments];
            Ptr<Sample> samples[numSamples];
            Ptr<Pattern> patterns[numPatterns];
        }
    };
}

it::Module module @ 0x0;