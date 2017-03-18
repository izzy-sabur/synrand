//
//  SynRandUtil.h
//  SynRand
//
//  Created by Penguin on 2/21/17.
//  Copyright (c) 2017 Penguin. All rights reserved.
//

#ifndef SynRand_SynRandUtil_h
#define SynRand_SynRandUtil_h

#define NUM_PARTIALS 4

enum {
    kGlobalVolume =0,
    //Add your parameters here...
    kNumGlobalParams
};

enum {
    kPartFreqMod = 0,
    kPartVolMod,
    kPartEnabledBool,
    kPartLFOFreq,
    kPartWaveType,
    kNumPartParams
};

enum {
    kWT_SINE = 0,
    kWT_SAW,
    kWT_SQUARE,
    kWT_TRI,
    kWT_MAXNUM
};

#define NUM_PARAMS (kNumGlobalParams + (kNumPartParams * NUM_PARTIALS))
#endif
