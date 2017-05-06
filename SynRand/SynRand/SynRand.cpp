/*
 
     File: SynRand.cpp
 Abstract: Audio Unit Music Instrument class implementation.
  Version: 1.0.1
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Inc. ("Apple") in consideration of your agreement to the following
 terms, and your use, installation, modification or redistribution of
 this Apple software constitutes acceptance of these terms.  If you do
 not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc. may
 be used to endorse or promote products derived from the Apple Software
 without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2012 Apple Inc. All Rights Reserved.
 
 
*/

#include "SynRand.h"
#include "string.h"

static const UInt32 kMaxActiveNotes = 8;

const double twopi = 2.0 * 3.1415926535;

inline double pow5(double x) {double x2 = x*x; return x2*x2*x;}
#define TO_MILLISECONDS 1000.0f
double squareSound(double pos)
{
    double s1 = pos/twopi;
    s1 = floorf(s1 + .5f);
    s1 *= 2;
    s1 -= 1;
    return s1;
}

double sawSound(double pos)
{
    double s1 = pos/twopi;
    s1 *= 2.0f;
    s1 -= 1;
    return s1;
}

double triSound(double pos)
{
    double s1 = pos/twopi;
    s1 = fabs(s1 - .5);
    s1 *= 4;
    s1 = 1 - s1;
    return s1;
}

double sinSound(double x)
{
    return pow5(sin(x));
}

double(*waveformFunc[kWT_MAXNUM])(double) = {sinSound,sawSound,squareSound,triSound};
#pragma mark SynRand Methods
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// COMPONENT_ENTRY(SynRand) deprecated on MacOS X 10.7 see TN2276 

AUDIOCOMPONENT_ENTRY(AUMusicDeviceFactory, SynRand)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	SynRand::SynRand
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SynRand::SynRand(AudioUnit component)
	: AUMonotimbralInstrumentBase(component,0,1)
{
	CreateElements();
    
	Globals()->UseIndexedParameters(kNumGlobalParams + kNumPartParams * NUM_PARTIALS);
	Globals()->SetParameter(kGlobalVolume, kDefaultValue_GlobalVolume );
    
    for(int i = 0; i < NUM_PARTIALS; i++)
    {
        Globals()->SetParameter(kNumGlobalParams + kPartFreqOffset + (kNumPartParams * i), SYRD_PARAM_INFO[kPartFreqOffset].mDefaultVal);
        Globals()->SetParameter(kNumGlobalParams + kPartVolMod + (kNumPartParams * i), SYRD_PARAM_INFO[kPartVolMod].mDefaultVal);
        
        if(i == 0)
            Globals()->SetParameter(kNumGlobalParams + kPartEnabledBool + (kNumPartParams * i), 1);
        else
            Globals()->SetParameter(kNumGlobalParams + kPartEnabledBool + (kNumPartParams * i), SYRD_PARAM_INFO[kPartEnabledBool].mDefaultVal);
        
        Globals()->SetParameter(kNumGlobalParams + kPartLFOFreq + (kNumPartParams * i),     SYRD_PARAM_INFO[kPartLFOFreq].mDefaultVal);
        Globals()->SetParameter(kNumGlobalParams + kPartWaveType + (kNumPartParams * i),    SYRD_PARAM_INFO[kPartWaveType].mDefaultVal);
        Globals()->SetParameter(kNumGlobalParams + kPartATKTime + (kNumPartParams * i),     SYRD_PARAM_INFO[kPartATKTime].mDefaultVal);
        Globals()->SetParameter(kNumGlobalParams + kPartDECTime + (kNumPartParams * i),     SYRD_PARAM_INFO[kPartDECTime].mDefaultVal);
        Globals()->SetParameter(kNumGlobalParams + kPartSUSLevel + (kNumPartParams * i),    SYRD_PARAM_INFO[kPartSUSLevel].mDefaultVal);
        Globals()->SetParameter(kNumGlobalParams + kPartRELTime + (kNumPartParams * i),     SYRD_PARAM_INFO[kPartRELTime].mDefaultVal);
        Globals()->SetParameter(kNumGlobalParams + kPartFreqScale + (kNumPartParams * i),   SYRD_PARAM_INFO[kPartFreqScale].mDefaultVal);
        Globals()->SetParameter(kNumGlobalParams + kPartFreqATKTime + (kNumPartParams * i), SYRD_PARAM_INFO[kPartFreqATKTime].mDefaultVal);
        Globals()->SetParameter(kNumGlobalParams + kPartFreqDECTime + (kNumPartParams * i), SYRD_PARAM_INFO[kPartFreqDECTime].mDefaultVal);
        Globals()->SetParameter(kNumGlobalParams + kPartFreqSUSLevel + (kNumPartParams * i), SYRD_PARAM_INFO[kPartFreqSUSLevel].mDefaultVal);
        Globals()->SetParameter(kNumGlobalParams + kPartFreqRELTime + (kNumPartParams * i), SYRD_PARAM_INFO[kPartFreqRELTime].mDefaultVal);
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	SynRand::~SynRand
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SynRand::~SynRand()
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	SynRand::Cleanup
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SynRand::Cleanup()
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	SynRand::Initialize
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus SynRand::Initialize()
{
    AUMonotimbralInstrumentBase::Initialize();
    SetNotes(kNumNotes, kMaxActiveNotes, mTestNotes, sizeof(TestNote));
    
    return noErr;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	SynRand::GetParameterValueStrings
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus			SynRand::GetParameterValueStrings(AudioUnitScope		inScope,
                                                                AudioUnitParameterID	inParameterID,
                                                                CFArrayRef *		outStrings)
{
    
    if(inScope == kAudioUnitScope_Global)
    {
        if(!outStrings)
            return noErr;
        if(inParameterID < kNumGlobalParams)
        {
            return noErr;
        }
        else
        {
            // otherwise, figure out which of the partial parameters it is
            int hold = inParameterID - kNumGlobalParams;
            hold %= kNumPartParams;
            
            switch(hold)
            {
                case kPartWaveType:
                    CFStringRef arr[] = {
                        CFSTR("Sine"),
                        CFSTR("Square"),
                        CFSTR("Saw"),
                        CFSTR("Triangle")
                    };
                    *outStrings = CFArrayCreate(kCFAllocatorDefault, (const void **)arr, 4, NULL);
                    return noErr;
            }
        }
    }
    return kAudioUnitErr_InvalidProperty;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	SynRand::CreateElement
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AUElement*          SynRand::CreateElement(	AudioUnitScope					scope,
                                            AudioUnitElement				element)
{
    switch (scope)
    {
        case kAudioUnitScope_Group :
            return new SynthGroupElement(this, element, new MidiControls);
        case kAudioUnitScope_Part :
            return new SynthPartElement(this, element);
        default :
            return AUBase::CreateElement(scope, element);
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	SynRand::GetParameterInfo
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus			SynRand::GetParameterInfo(AudioUnitScope		inScope,
                                                        AudioUnitParameterID	inParameterID,
                                                        AudioUnitParameterInfo	&outParameterInfo )
{
	OSStatus result = noErr;

	outParameterInfo.flags = 	kAudioUnitParameterFlag_IsWritable
						|		kAudioUnitParameterFlag_IsReadable;
    
    if (inScope == kAudioUnitScope_Global) {
        
        // if this is a global parameter
        if(inParameterID < kNumGlobalParams)
        {
            // switch and declare it as normal
            switch(inParameterID)
            {
                case kGlobalVolume:
                    AUBase::FillInParameterName (outParameterInfo, kGlobalVolumeName, false);
                    outParameterInfo.unit = kAudioUnitParameterUnit_LinearGain;
                    outParameterInfo.minValue = 0.0;
                    outParameterInfo.maxValue = 1;
                    outParameterInfo.defaultValue = kDefaultValue_GlobalVolume;
                    break;
                default:
                    result = kAudioUnitErr_InvalidParameter;
                    break;
            }
        }
        else
        {
            // otherwise, figure out which of the partial parameters it is
            int hold = inParameterID - kNumGlobalParams;
            int numGroup = (hold / kNumPartParams) + 1;
            hold %= kNumPartParams;
            
            if((hold < kNumPartParams) && !(hold < 0))
            {
                AUBase::FillInParameterName (outParameterInfo, SYRD_PARAM_INFO[hold].mName, false);
                outParameterInfo.unit = SYRD_PARAM_INFO[hold].mUnitType;
                outParameterInfo.minValue = SYRD_PARAM_INFO[hold].mMinVal;
                outParameterInfo.maxValue = SYRD_PARAM_INFO[hold].mMaxVal;
                outParameterInfo.clumpID = numGroup;
                outParameterInfo.defaultValue = SYRD_PARAM_INFO[hold].mDefaultVal;
                AUBase::HasClump(outParameterInfo, numGroup);
            }
            else
            {
                result = kAudioUnitErr_InvalidParameter;
            }
        }
	}
    else
    {
        result = kAudioUnitErr_InvalidParameter;
    }
    


	return result;
}


// START COCOA UI::
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	SynRand::GetPropertyInfo
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus			SynRand::GetPropertyInfo (AudioUnitPropertyID	inID,
                                                        AudioUnitScope		inScope,
                                                        AudioUnitElement	inElement,
                                                        UInt32 &		outDataSize,
                                                        Boolean &		outWritable)
{
	if (inScope == kAudioUnitScope_Global) 
	{
		switch (inID) 
		{
			case kAudioUnitProperty_CocoaUI:
				outWritable = false;
				outDataSize = sizeof (AudioUnitCocoaViewInfo);
				return noErr;
            case kAudioUnitProperty_ParameterClumpName:
                outWritable = false;
                outDataSize = sizeof(AudioUnitParameterIDName);
                return noErr;
					
		}
	}

	return AUMonotimbralInstrumentBase::GetPropertyInfo (inID, inScope, inElement, outDataSize, outWritable);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	SynRand::GetProperty
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus			SynRand::GetProperty(	AudioUnitPropertyID inID,
															AudioUnitScope 		inScope,
															AudioUnitElement 	inElement,
															void *				outData )
{
	if (inScope == kAudioUnitScope_Global) 
	{
		switch (inID) 
		{
			case kAudioUnitProperty_CocoaUI:
			{
				// Look for a resource in the main bundle by name and type.
				CFBundleRef bundle = CFBundleGetBundleWithIdentifier( CFSTR("com.PENG.audiounit.SynRand") );
				
				if (bundle == NULL) return noErr;
                
				CFURLRef bundleURL = CFBundleCopyResourceURL( bundle, 
                    CFSTR("SynRand_CocoaViewFactory"), 
                    CFSTR("bundle"), 
                    NULL);
                
                if (bundleURL == NULL) return noErr;

				AudioUnitCocoaViewInfo cocoaInfo;
				cocoaInfo.mCocoaAUViewBundleLocation = bundleURL;
				cocoaInfo.mCocoaAUViewClass[0] = CFStringCreateWithCString(NULL, "SynRand_CocoaViewFactory", kCFStringEncodingUTF8);
				
				*((AudioUnitCocoaViewInfo *)outData) = cocoaInfo;
				
				return noErr;
			}
            case kAudioUnitProperty_ParameterClumpName:
            {
                AudioUnitParameterNameInfo &info = *((AudioUnitParameterNameInfo *) outData);
                
                info.outName = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("Partial #%d"), info.inID);
                
                return noErr;
            }
		}
	}

	return AUMonotimbralInstrumentBase::GetProperty (inID, inScope, inElement, outData);
}

// END COCOA UI

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#pragma mark TestNote Methods

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	TestNote::Release
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void                TestNote::Release(UInt32 inFrame)
{
    SynthNote::Release(inFrame);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	TestNote::FastRelease
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void                TestNote::FastRelease(UInt32 inFrame)
{
    SynthNote::Release(inFrame);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	TestNote::Kill
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void                TestNote::Kill(UInt32 inFrame)
{
    SynthNote::Kill(inFrame);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	TestNote::Render
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus           TestNote::Render(UInt64 inAbsoluteSampleFrame, UInt32 inNumFrames, AudioBufferList **inBufferList, UInt32 inOutBusCount)
{
    float *left, *right;
    
    float globalVol = GetGlobalParameter(kGlobalVolume);
    float volMod[NUM_PARTIALS];
    float lfoFreq[NUM_PARTIALS];
    bool partEnabled[NUM_PARTIALS];
    float partMod[NUM_PARTIALS];
    float pFreq[NUM_PARTIALS];
    int pWaveType[NUM_PARTIALS];
    float partATKSlope[NUM_PARTIALS];
    float partDECSlope[NUM_PARTIALS];
    float partSUSLevel[NUM_PARTIALS];
    float partRELSlope[NUM_PARTIALS];
    float partFreqScale[NUM_PARTIALS];
    float partFreqATKSlope[NUM_PARTIALS];
    float partFreqDECSlope[NUM_PARTIALS];
    float partFreqSUSLevel[NUM_PARTIALS];
    float partFreqRELSlope[NUM_PARTIALS];
    float curSlope[NUM_PARTIALS];
    bool releasing[NUM_PARTIALS];
    bool ended[NUM_PARTIALS];
    
    // only writes into first bus
    const int bus0 = 0;
    int numChans = inBufferList[bus0]->mNumberBuffers;
    
    // can only write to mono or stereo
    if(numChans > 2)
        return -1;
    
    left = (float*)inBufferList[bus0]->mBuffers[0].mData;
    right = numChans == 2 ? (float*)inBufferList[bus0]->mBuffers[1].mData : nullptr;
    
    double sampleRate = SampleRate();
    double freq = Frequency() * (twopi / sampleRate);
    
    // init our ridiculous panoply of partial values
    for(int i = 0; i < NUM_PARTIALS; i++)
    {
        partMod[i] = GetGlobalParameter(kNumGlobalParams + (kPartFreqOffset) + (i * kNumPartParams));
        volMod[i] = GetGlobalParameter(kNumGlobalParams + (kPartVolMod) + (i * kNumPartParams));
        lfoFreq[i] = GetGlobalParameter(kNumGlobalParams + (kPartLFOFreq) + (i * kNumPartParams));
        partEnabled[i] = GetGlobalParameter(kNumGlobalParams + (kPartEnabledBool) + (i * kNumPartParams));
        pWaveType[i] = GetGlobalParameter(kNumGlobalParams + (kPartWaveType) + (i * kNumPartParams));
        partATKSlope[i] = maxamp / ((GetGlobalParameter(kNumGlobalParams + (kPartATKTime) + (i * kNumPartParams)) / TO_MILLISECONDS) * sampleRate);
        partSUSLevel[i] = GetGlobalParameter(kNumGlobalParams + (kPartSUSLevel) + (i * kNumPartParams)) * maxamp;
        partDECSlope[i] = (partSUSLevel[i] - maxamp) / ((GetGlobalParameter(kNumGlobalParams + (kPartDECTime) + (i * kNumPartParams)) / TO_MILLISECONDS) * sampleRate);
        partRELSlope[i] = -maxamp / ((GetGlobalParameter(kNumGlobalParams + (kPartRELTime) + (i * kNumPartParams)) / TO_MILLISECONDS) * sampleRate);
        partFreqScale[i] = GetGlobalParameter(kNumGlobalParams + (kPartFreqScale) + (i * kNumPartParams));
        partFreqATKSlope[i] = 1 / ((GetGlobalParameter(kNumGlobalParams + (kPartFreqATKTime) + (i * kNumPartParams)) / TO_MILLISECONDS) * sampleRate);
        partFreqSUSLevel[i] = GetGlobalParameter(kNumGlobalParams + (kPartFreqSUSLevel) + (i * kNumPartParams));
        partFreqDECSlope[i] = (partSUSLevel[i] - 1) / ((GetGlobalParameter(kNumGlobalParams + (kPartFreqDECTime) + (i * kNumPartParams)) / TO_MILLISECONDS) * sampleRate);
        partFreqRELSlope[i] = -1 / ((GetGlobalParameter(kNumGlobalParams + (kPartFreqRELTime) + (i * kNumPartParams)) / TO_MILLISECONDS) * sampleRate);
        float semitoneOffset = pow(2, partMod[i] / 12);
        pFreq[i] = semitoneOffset * freq;
        curSlope[i] = 0;
        releasing[i] = true;
        ended[i] = false;
    }

    // based on the note state, set our movement slope
    switch(GetState())
    {
        case kNoteState_Attacked:
        case kNoteState_Sostenutoed:
        case kNoteState_ReleasedButSostenutoed:
        case kNoteState_ReleasedButSustained:
        {
            for(int i =0; i < NUM_PARTIALS; i++)
            {
                releasing[i] = false;
            }
        }
            break;
        
        case kNoteState_Released:
        {
            for(int i =0; i < NUM_PARTIALS; i++)
            {
                curSlope[i] = partRELSlope[i];
                releasing[i] = true;
            }
        }
            break;
            
        case kNoteState_FastReleased:
        {
            for(int i =0; i < NUM_PARTIALS; i++)
            {
                curSlope[i] = fast_dn_slope;
                releasing[i] = true;
            }
        }
            break;
        default:
            break;
    }
    
    UInt32 endFrame = 0xFFFFFFFF;
    for (UInt32 frame = 0; frame < inNumFrames; ++frame)
    {
        float out = 0;
        float numVoices = 0;
        bool allVoicesEnded = true;
        for(int i = 0; i < NUM_PARTIALS; i++)
        {
            // if we're not releasing yet
            if(!releasing[i])
            {
                allVoicesEnded = false;
                
                ///////////////// AMPLITUDE ADSR /////////////////////
                // if we haven't hit unity yet
                if(!hitUnity[i])
                {
                    // if we're not at max yet
                    if (partAmp[i] < maxamp)
                    {
                        partAmp[i] += partATKSlope[i]; // ATTACK
                        
                        // and see if we got to unity yet
                        if(partAmp[i] >= maxamp)
                            hitUnity[i] = true;
                    }
                }
                else // if we have
                {
                    // if we're not at sus level yet
                    if(partAmp[i] > partSUSLevel[i])
                        partAmp[i] += partDECSlope[i]; // DECAY
                    
                    // but if we are, SUSTAIN
                }
                
                ///////////////// FREQUENCY ADSR /////////////////////
                if(!hitFreqUnity[i])
                {
                    // if we're not at max yet
                    if (partFreqAmp[i] < 1)
                    {
                        partFreqAmp[i] += partFreqATKSlope[i]; // ATTACK
                        
                        // and see if we got to unity yet
                        if(partFreqAmp[i] >= 1)
                            hitFreqUnity[i] = true;
                    }
                }
                else // if we have
                {
                    // if we're not at sus level yet
                    if(partFreqAmp[i] > partFreqSUSLevel[i])
                        partFreqAmp[i] += partFreqDECSlope[i]; // DECAY
                    
                    // but if we are, SUSTAIN
                }
            }
            else // if we are releasing
            {
                // if there's more releasing to be done
                if (partAmp[i] > 0)
                {
                    partAmp[i] += curSlope[i]; // RELEASE
                    allVoicesEnded = false;
                }
                else if(!ended[i]) // if there isn't more release to do and we haven't ended yet
                    ended[i] = true; // end
                
                // if there's more releasing to be done
                if (partFreqAmp[i] > 0)
                {
                    partFreqAmp[i] += partFreqRELSlope[i]; // RELEASE
                }
            }
            
            if(partEnabled[i])
            {
                double waveValue = waveformFunc[pWaveType[i]](partPhase[i]);
                out += waveValue * ((lfoFreq[i] != 0) ? sin(lfoPhase[i]) : 1) * partAmp[i] * globalVol * volMod[i];
                numVoices++;
            }
        }
        
        if(numVoices > 0)
            out /= numVoices;
        
        // increment the phase by the current (angular) frequency
        for(int i = 0; i < NUM_PARTIALS; i++)
        {
            partPhase[i] += pFreq[i] * pow(2, partFreqAmp[i] * partFreqScale[i] / 12);
            lfoPhase[i] += lfoFreq[i] * (twopi / sampleRate);
        }
        
        // remember to wrap phase around
        for(int i = 0; i < NUM_PARTIALS; i++)
        {
            if (partPhase[i] > twopi)
                partPhase[i] -= twopi;
            
            if (lfoPhase[i] > twopi)
                lfoPhase[i] -= twopi;
        }
        
        if((allVoicesEnded) && (endFrame == 0xFFFFFFFF))
            endFrame = inNumFrames - 1;
        
        // then write to left and right
        left[frame] += out;
        if(right)
            right[frame] += out;
    }

    // if there was an end frame, note it
    if(endFrame != 0xFFFFFFFF)
        NoteEnded(endFrame);
    return noErr;
}




