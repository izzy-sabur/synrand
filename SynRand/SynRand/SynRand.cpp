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
        Globals()->SetParameter(kNumGlobalParams + kPartFreqMod + (kNumPartParams * i), kDefaultValue_PartFreqMod);
        Globals()->SetParameter(kNumGlobalParams + kPartVolMod + (kNumPartParams * i), kDefaultValue_PartVolMod);
        Globals()->SetParameter(kNumGlobalParams + kPartEnabledBool + (kNumPartParams * i), kDefaultValue_PartEnabledBool);
        Globals()->SetParameter(kNumGlobalParams + kPartLFOFreq + (kNumPartParams * i), kDefaultValue_PartLFOFreq);
        Globals()->SetParameter(kNumGlobalParams + kPartWaveType + (kNumPartParams * i), kDefaultValue_PartWaveType);
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
            
            switch(hold)
            {
                case kPartFreqMod:
                    AUBase::FillInParameterName (outParameterInfo, kPartFreqModName, false);
                    outParameterInfo.unit = kAudioUnitParameterUnit_LinearGain;
                    outParameterInfo.minValue = 0.0;
                    outParameterInfo.maxValue = 16;
                    outParameterInfo.clumpID = numGroup;
                    outParameterInfo.defaultValue = kDefaultValue_PartFreqMod;
                    AUBase::HasClump(outParameterInfo, numGroup);
                    break;
                case kPartVolMod:
                    AUBase::FillInParameterName (outParameterInfo, kPartVolModName, false);
                    outParameterInfo.unit = kAudioUnitParameterUnit_LinearGain;
                    outParameterInfo.minValue = 0.0;
                    outParameterInfo.maxValue = 2;
                    outParameterInfo.clumpID = numGroup;
                    outParameterInfo.defaultValue = kDefaultValue_PartVolMod;
                    AUBase::HasClump(outParameterInfo, numGroup);
                    break;
                case kPartEnabledBool:
                    AUBase::FillInParameterName (outParameterInfo, kPartEnabledBoolName, false);
                    outParameterInfo.unit = kAudioUnitParameterUnit_Boolean;
                    outParameterInfo.minValue = 0;
                    outParameterInfo.maxValue = 1;
                    outParameterInfo.clumpID = numGroup;
                    outParameterInfo.defaultValue = kDefaultValue_PartEnabledBool;
                    AUBase::HasClump(outParameterInfo, numGroup);
                    break;
                case kPartLFOFreq:
                    AUBase::FillInParameterName (outParameterInfo, kPartLFOFreqName, false);
                    outParameterInfo.unit = kAudioUnitParameterUnit_LinearGain;
                    outParameterInfo.minValue = 0.0;
                    outParameterInfo.maxValue = 60;
                    outParameterInfo.clumpID = numGroup;
                    outParameterInfo.defaultValue = kDefaultValue_PartLFOFreq;
                    AUBase::HasClump(outParameterInfo, numGroup);
                    break;
                case kPartWaveType:
                    AUBase::FillInParameterName(outParameterInfo, kPartWaveTypeName, false);
                    outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
                    outParameterInfo.minValue = kWT_SINE;
                    outParameterInfo.maxValue = kWT_MAXNUM - 1;
                    outParameterInfo.clumpID = numGroup;
                    outParameterInfo.defaultValue = kDefaultValue_PartWaveType;
                    AUBase::HasClump(outParameterInfo, numGroup);
                    break;
                default:
                    result = kAudioUnitErr_InvalidParameter;
                    break;
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
    
    for(int i = 0; i < NUM_PARTIALS; i++)
    {
        partMod[i] = GetGlobalParameter(kNumGlobalParams + (kPartFreqMod) + (i * kNumPartParams));
        volMod[i] = GetGlobalParameter(kNumGlobalParams + (kPartVolMod) + (i * kNumPartParams));
        lfoFreq[i] = GetGlobalParameter(kNumGlobalParams + (kPartLFOFreq) + (i * kNumPartParams));
        partEnabled[i] = GetGlobalParameter(kNumGlobalParams + (kPartEnabledBool) + (i * kNumPartParams));
        pWaveType[i] = GetGlobalParameter(kNumGlobalParams + (kPartWaveType) + (i * kNumPartParams));
        pFreq[i] = partMod[i] * freq;
    }

    float slopeAmount = 0.0;
    switch(GetState())
    {
        case kNoteState_Attacked:
        case kNoteState_Sostenutoed:
        case kNoteState_ReleasedButSostenutoed:
        case kNoteState_ReleasedButSustained:
        {
            slopeAmount = up_slope;
        }
            break;
        
        case kNoteState_Released:
        {
            slopeAmount = dn_slope;
        }
            break;
            
        case kNoteState_FastReleased:
        {
            slopeAmount = fast_dn_slope;
        }
            break;
        default:
            break;
    }
    
    UInt32 endFrame = 0xFFFFFFFF;
    bool slopeUp = slopeAmount > 0;
    for (UInt32 frame = 0; frame < inNumFrames; ++frame)
    {
        // if we're going up and below max, go up
        if ((amp < maxamp) && slopeUp)
            amp += slopeAmount;
        
        // if we're going down and above 0, go down
        if(!slopeUp)
        {
            if (amp > 0)
                amp += slopeAmount;
            else if (endFrame == 0xFFFFFFFF) // if there isn't more decay to do, mark the end frame
                endFrame = frame;
        }
        
        float out = 0;
        float numVoices = 0;
        
        for(int i = 0; i < NUM_PARTIALS; i++)
        {
            if(partMod[i] > 0 && partEnabled[i])
            {
                double waveValue = waveformFunc[pWaveType[i]](partPhase[i]);
                out += waveValue * ((lfoFreq[i] != 0) ? sin(lfoPhase[i]) : 1) * amp * globalVol * volMod[i];
                numVoices++;
            }
        }
        
        if(numVoices > 0)
            out /= numVoices;
        
        // increment the phase by the current (angular) frequency
        for(int i = 0; i < NUM_PARTIALS; i++)
        {
            partPhase[i] += pFreq[i];
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




