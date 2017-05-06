//
//  SynRandUtil.h
//  SynRand
//
//  Created by Penguin on 2/21/17.
//  Copyright (c) 2017 Penguin. All rights reserved.
//

#ifndef SynRand_SynRandUtil_h
#define SynRand_SynRandUtil_h

#define NUM_PARTIALS 16

enum {
    kGlobalVolume =0,
    //Add your parameters here...
    kNumGlobalParams
};

enum {
    kPartFreqOffset = 0,
    kPartVolMod,
    kPartEnabledBool,
    kPartLFOFreq,
    kPartWaveType,
    kPartATKTime,
    kPartDECTime,
    kPartSUSLevel,
    kPartRELTime,
    kPartFreqScale,
    kPartFreqATKTime,
    kPartFreqDECTime,
    kPartFreqSUSLevel,
    kPartFreqRELTime,
    kNumPartParams
};

enum {
    kWT_SINE = 0,
    kWT_SAW,
    kWT_SQUARE,
    kWT_TRI,
    kWT_MAXNUM
};

struct ParamInfo
{
    CFStringRef mName;
    AudioUnitParameterUnit mUnitType;
    float mMinVal;
    float mMaxVal;
    float mDefaultVal;
} typedef ParamInfo;


static ParamInfo SYRD_PARAM_INFO[kNumPartParams] =
{
    {CFSTR("Frequency Offset"),             kAudioUnitParameterUnit_RelativeSemiTones,  -96, 96, 0},
    {CFSTR("Volume Modifier"),              kAudioUnitParameterUnit_LinearGain,         0, 1, 1},
    {CFSTR("Enabled State"),                kAudioUnitParameterUnit_Boolean,            0, 1, 0},
    {CFSTR("Low Frequency Oscillator"),     kAudioUnitParameterUnit_Hertz,              0, 60, 0},
    {CFSTR("Waveform Type"),                kAudioUnitParameterUnit_Indexed,            kWT_SINE, kWT_MAXNUM - 1, kWT_SINE},
    {CFSTR("Attack Time"),                  kAudioUnitParameterUnit_Milliseconds,       .1, 10000, 25},
    {CFSTR("Decay Time"),                   kAudioUnitParameterUnit_Milliseconds,       .1, 10000, 25},
    {CFSTR("Sustain Level"),                kAudioUnitParameterUnit_LinearGain,         0, 1, .5},
    {CFSTR("Release Time"),                 kAudioUnitParameterUnit_Milliseconds,       .1, 10000, 25},
    {CFSTR("Frequency Scale"),              kAudioUnitParameterUnit_RelativeSemiTones,  -96, 96, 0}, // freq scale
    {CFSTR("Frequency Attack Time"),        kAudioUnitParameterUnit_Milliseconds,       .1, 10000, 25}, // freq attack
    {CFSTR("Frequency Decay Time"),         kAudioUnitParameterUnit_Milliseconds,       .1, 10000, 25}, // freq decay
    {CFSTR("Frequency Sustain Level"),      kAudioUnitParameterUnit_LinearGain,         0, 1, .5}, // freq sus
    {CFSTR("Frequency Release Time"),       kAudioUnitParameterUnit_Milliseconds,       .1, 10000, 25}, // freq release
};
#define NUM_PARAMS (kNumGlobalParams + (kNumPartParams * NUM_PARTIALS))
#endif
