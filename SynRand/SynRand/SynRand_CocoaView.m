/*
 
     File: SynRand_CocoaView.m
 Abstract: Audio Unit Cocoa View imlementation.
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

#import "SynRand_CocoaView.h"

#pragma mark ____ LISTENER CALLBACK DISPATCHER ____
void ParameterListenerDispatcher (void *inRefCon, void *inObject, const AudioUnitParameter *inParameter, Float32 inValue) {
	SynRand_CocoaView *SELF = (SynRand_CocoaView *)inRefCon;
    
    [SELF parameterListener:inObject parameter:inParameter value:inValue];
}

NSString *SynRand_GestureSliderMouseDownNotification = @"CAGestureSliderMouseDownNotification";
NSString *SynRand_GestureSliderMouseUpNotification = @"CAGestureSliderMouseUpNotification";

@implementation SynRand_GestureSlider

/*	We create our own custom subclass of NSSlider so we can do begin/end gesture notification
	We cannot override mouseUp: because it will never be called. Instead we do a clever trick in mouseDown to send mouseUp notifications */
- (void)mouseDown:(NSEvent *)inEvent {
	[[NSNotificationCenter defaultCenter] postNotificationName: SynRand_GestureSliderMouseDownNotification object: self];
	
	[super mouseDown: inEvent];	// this call does not return until mouse tracking is complete
								// once tracking is done, we know the mouse has been released, so we can send our mouseup notification

	[[NSNotificationCenter defaultCenter] postNotificationName: SynRand_GestureSliderMouseUpNotification object: self];
}
	
@end


@implementation SynRand_CocoaView

-(instancetype)initWithFrame:(NSRect)frameRect
{
    if (!(self = [super initWithFrame:frameRect]))
        return nil;
    
    mNumParameters = (kNumGlobalParams + (kNumPartParams * NUM_PARTIALS));
    //mParameter = malloc(sizeof(mParameter) * mNumParameters);
    
    if(!mParameter)
    {
        [self release];
        return nil;
    }
    
    return self;
}

#pragma mark ____ (INIT /) DEALLOC ____
- (void)dealloc {
    [self removeListeners];
    //free(mParameter);
    [super dealloc];
}

#pragma mark ____ PUBLIC FUNCTIONS ____
- (void)setAU:(AudioUnit)inAU {
	// remove previous listeners
	if (mAU) [self removeListeners];
	mAU = inAU;
    
    for(int i = 0; i < mNumParameters; i++)
    {
        mParameter[i].mAudioUnit = inAU;
        mParameter[i].mParameterID = i;
        mParameter[i].mScope = kAudioUnitScope_Global;
        mParameter[i].mElement = 0;
    }

	// add new listeners
	[self addListeners];
	
	// initial setup
	[self synchronizeUIWithParameterValues];
}

#pragma mark ____ INTERFACE ACTIONS ____
- (IBAction)iaGlobalVolChanged:(id)sender
{
    float floatValue = [sender floatValue];
    
    OSStatus result = AUParameterSet(mParameterListener, sender, &mParameter[0], (Float32)floatValue, 0);
    NSAssert(result == noErr, @"[SynRand_CocoaView iaParam1Changed:] AUParameterSet()");
    
    if (sender == uiGlobalVolSlider) {
        [uiGlobalVolField setFloatValue:floatValue];
    } else {
        [uiGlobalVolSlider setFloatValue:floatValue];
    }
}

- (IBAction)iaPartialCellChanged:(id)sender
{
    mTargetPartialGroup = uiPartSelectCells.selectedSegment;
    [uiPartNumLabel setStringValue:[[NSNumber numberWithFloat:mTargetPartialGroup + 1] stringValue]];
    [self synchronizeUIWithParameterValues];
    
    // if this partial is enabled
    [self updateVisuals];
}

- (IBAction)iaPartEnabledChanged:(id)sender
{
    bool onVal = uiPartEnabledBtn.state;
    
    // determine the parameter number based off which group we're currently focusing on
    int paramNum = kNumGlobalParams + (mTargetPartialGroup * kNumPartParams) + kPartEnabledBool;
    OSStatus result = AUParameterSet(mParameterListener, sender, &mParameter[paramNum], (Float32)onVal, 0);
    
    NSAssert(result == noErr, @"[SynRand_CocoaView iaPartEnabledChanged:] AUParameterSet()");
    
    [self updateVisuals];
}

- (IBAction)iaPartFreqChanged:(id)sender
{
    float floatValue = [sender floatValue];
    
    // check to make sure it's within the range
    if(floatValue < SYRD_PARAM_INFO[kPartFreqOffset].mMinVal)
        floatValue = SYRD_PARAM_INFO[kPartFreqOffset].mMinVal;
    
    if(floatValue > SYRD_PARAM_INFO[kPartFreqOffset].mMaxVal)
        floatValue = SYRD_PARAM_INFO[kPartFreqOffset].mMaxVal;
    
    // determine the parameter number based off which group we're currently focusing on
    int paramNum = kNumGlobalParams + (mTargetPartialGroup * kNumPartParams) + kPartFreqOffset;
    OSStatus result = AUParameterSet(mParameterListener, sender, &mParameter[paramNum], (Float32)floatValue, 0);
    
    NSAssert(result == noErr, @"[SynRand_CocoaView iaPartFreqChanged:] AUParameterSet()");
    
    if (sender == uiPartFreqSlider) {
        [uiPartFreqField setFloatValue:floatValue];
    } else {
        [uiPartFreqSlider setFloatValue:floatValue];
    }
}

- (IBAction)iaPartLFOChanged:(id)sender
{
    float floatValue = [sender floatValue];
    
    // check to make sure it's within the range
    if(floatValue < SYRD_PARAM_INFO[kPartLFOFreq].mMinVal)
        floatValue = SYRD_PARAM_INFO[kPartLFOFreq].mMinVal;
    
    if(floatValue > SYRD_PARAM_INFO[kPartLFOFreq].mMaxVal)
        floatValue = SYRD_PARAM_INFO[kPartLFOFreq].mMaxVal;
    
    // determine the parameter number based off which group we're currently focusing on
    int paramNum = kNumGlobalParams + (mTargetPartialGroup * kNumPartParams) + kPartLFOFreq;
    OSStatus result = AUParameterSet(mParameterListener, sender, &mParameter[paramNum], (Float32)floatValue, 0);
    
    NSAssert(result == noErr, @"[SynRand_CocoaView iaPartLFOChanged:] AUParameterSet()");
    
    if (sender == uiPartLFOSlider) {
        [uiPartLFOFreqField setFloatValue:floatValue];
    } else {
        [uiPartLFOSlider setFloatValue:floatValue];
    }
}

- (IBAction)iaPartVolChanged:(id)sender
{
    float floatValue = [sender floatValue];
    
    // check to make sure it's within the range
    if(floatValue < SYRD_PARAM_INFO[kPartVolMod].mMinVal)
        floatValue = SYRD_PARAM_INFO[kPartVolMod].mMinVal;
    
    if(floatValue > SYRD_PARAM_INFO[kPartVolMod].mMaxVal)
        floatValue = SYRD_PARAM_INFO[kPartVolMod].mMaxVal;
    
    // determine the parameter number based off which group we're currently focusing on
    int paramNum = kNumGlobalParams + (mTargetPartialGroup * kNumPartParams) + kPartVolMod;
    OSStatus result = AUParameterSet(mParameterListener, sender, &mParameter[paramNum], (Float32)floatValue, 0);
    
    NSAssert(result == noErr, @"[SynRand_CocoaView iaPartVolChanged:] AUParameterSet()");
    
    if (sender == uiPartVolSlider) {
        [uiPartVolField setFloatValue:floatValue];
    } else {
        [uiPartVolSlider setFloatValue:floatValue];
    }
}

- (IBAction)iaPartWaveformChanged:(id)sender
{
    int index = (int)uiWaveformList.selectedTag;
    
    // determine the parameter number based off which group we're currently focusing on
    int paramNum = kNumGlobalParams + (mTargetPartialGroup * kNumPartParams) + kPartWaveType;
    OSStatus result = AUParameterSet(mParameterListener, sender, &mParameter[paramNum], (Float32)index, 0);
    
    NSAssert(result == noErr, @"[SynRand_CocoaView iaPartVolChanged:] AUParameterSet()");
}

- (IBAction)iaPartAmpAttackChanged:(id)sender
{
    float floatValue = [sender floatValue];
    
    // check to make sure it's within the range
    if(floatValue < SYRD_PARAM_INFO[kPartATKTime].mMinVal)
        floatValue = SYRD_PARAM_INFO[kPartATKTime].mMinVal;
    
    if(floatValue > SYRD_PARAM_INFO[kPartATKTime].mMaxVal)
        floatValue = SYRD_PARAM_INFO[kPartATKTime].mMaxVal;
    
    // determine the parameter number based off which group we're currently focusing on
    int paramNum = kNumGlobalParams + (mTargetPartialGroup * kNumPartParams) + kPartATKTime;
    OSStatus result = AUParameterSet(mParameterListener, sender, &mParameter[paramNum], (Float32)floatValue, 0);
    
    NSAssert(result == noErr, @"[SynRand_CocoaView iaPartVolChanged:] AUParameterSet()");
    
    if (sender == uiAmpAttackSlider) {
        [uiAmpAttackField setFloatValue:floatValue];
    } else {
        [uiAmpAttackSlider setFloatValue:floatValue];
    }
    
    [self updateVisuals];
}

- (IBAction)iaPartAmpDecayChanged:(id)sender
{
    float floatValue = [sender floatValue];
    
    // check to make sure it's within the range
    if(floatValue < SYRD_PARAM_INFO[kPartDECTime].mMinVal)
        floatValue = SYRD_PARAM_INFO[kPartDECTime].mMinVal;
    
    if(floatValue > SYRD_PARAM_INFO[kPartDECTime].mMaxVal)
        floatValue = SYRD_PARAM_INFO[kPartDECTime].mMaxVal;
    
    // determine the parameter number based off which group we're currently focusing on
    int paramNum = kNumGlobalParams + (mTargetPartialGroup * kNumPartParams) + kPartDECTime;
    OSStatus result = AUParameterSet(mParameterListener, sender, &mParameter[paramNum], (Float32)floatValue, 0);
    
    NSAssert(result == noErr, @"[SynRand_CocoaView iaPartVolChanged:] AUParameterSet()");
    
    if (sender == uiAmpDecaySlider) {
        [uiAmpDecayField setFloatValue:floatValue];
    } else {
        [uiAmpDecaySlider setFloatValue:floatValue];
    }
    
    [self updateVisuals];
}

- (IBAction)iaPartAmpSustainChanged:(id)sender
{
    float floatValue = [sender floatValue];
    
    // check to make sure it's within the range
    if(floatValue < SYRD_PARAM_INFO[kPartSUSLevel].mMinVal)
        floatValue = SYRD_PARAM_INFO[kPartSUSLevel].mMinVal;
    
    if(floatValue > SYRD_PARAM_INFO[kPartSUSLevel].mMaxVal)
        floatValue = SYRD_PARAM_INFO[kPartSUSLevel].mMaxVal;
    
    // determine the parameter number based off which group we're currently focusing on
    int paramNum = kNumGlobalParams + (mTargetPartialGroup * kNumPartParams) + kPartSUSLevel;
    OSStatus result = AUParameterSet(mParameterListener, sender, &mParameter[paramNum], (Float32)floatValue, 0);
    
    NSAssert(result == noErr, @"[SynRand_CocoaView iaPartVolChanged:] AUParameterSet()");
    
    if (sender == uiAmpSustainSlider) {
        [uiAmpSustainField setFloatValue:floatValue];
    } else {
        [uiAmpSustainSlider setFloatValue:floatValue];
    }
    
    [self updateVisuals];
}

- (IBAction)iaPartAmpReleaseChanged:(id)sender
{
    float floatValue = [sender floatValue];
    
    // check to make sure it's within the range
    if(floatValue < SYRD_PARAM_INFO[kPartRELTime].mMinVal)
        floatValue = SYRD_PARAM_INFO[kPartRELTime].mMinVal;
    
    if(floatValue > SYRD_PARAM_INFO[kPartRELTime].mMaxVal)
        floatValue = SYRD_PARAM_INFO[kPartRELTime].mMaxVal;
    
    // determine the parameter number based off which group we're currently focusing on
    int paramNum = kNumGlobalParams + (mTargetPartialGroup * kNumPartParams) + kPartRELTime;
    OSStatus result = AUParameterSet(mParameterListener, sender, &mParameter[paramNum], (Float32)floatValue, 0);
    
    NSAssert(result == noErr, @"[SynRand_CocoaView iaPartVolChanged:] AUParameterSet()");
    
    if (sender == uiAmpReleaseSlider) {
        [uiAmpReleaseField setFloatValue:floatValue];
    } else {
        [uiAmpReleaseSlider setFloatValue:floatValue];
    }
    
    [self updateVisuals];
}

- (IBAction)iaPartFreqAttackChanged:(id)sender
{
    float floatValue = [sender floatValue];
    
    // check to make sure it's within the range
    if(floatValue < SYRD_PARAM_INFO[kPartFreqATKTime].mMinVal)
        floatValue = SYRD_PARAM_INFO[kPartFreqATKTime].mMinVal;
    
    if(floatValue > SYRD_PARAM_INFO[kPartFreqATKTime].mMaxVal)
        floatValue = SYRD_PARAM_INFO[kPartFreqATKTime].mMaxVal;
    
    // determine the parameter number based off which group we're currently focusing on
    int paramNum = kNumGlobalParams + (mTargetPartialGroup * kNumPartParams) + kPartFreqATKTime;
    OSStatus result = AUParameterSet(mParameterListener, sender, &mParameter[paramNum], (Float32)floatValue, 0);
    
    NSAssert(result == noErr, @"[SynRand_CocoaView iaPartVolChanged:] AUParameterSet()");
    
    if (sender == uiFreqAttackSlider) {
        [uiFreqAttackField setFloatValue:floatValue];
    } else {
        [uiFreqAttackSlider setFloatValue:floatValue];
    }
    
    [self updateVisuals];
}

- (IBAction)iaPartFreqDecayChanged:(id)sender
{
    float floatValue = [sender floatValue];
    
    // check to make sure it's within the range
    if(floatValue < SYRD_PARAM_INFO[kPartFreqDECTime].mMinVal)
        floatValue = SYRD_PARAM_INFO[kPartFreqDECTime].mMinVal;
    
    if(floatValue > SYRD_PARAM_INFO[kPartFreqDECTime].mMaxVal)
        floatValue = SYRD_PARAM_INFO[kPartFreqDECTime].mMaxVal;
    
    // determine the parameter number based off which group we're currently focusing on
    int paramNum = kNumGlobalParams + (mTargetPartialGroup * kNumPartParams) + kPartFreqDECTime;
    OSStatus result = AUParameterSet(mParameterListener, sender, &mParameter[paramNum], (Float32)floatValue, 0);
    
    NSAssert(result == noErr, @"[SynRand_CocoaView iaPartVolChanged:] AUParameterSet()");
    
    if (sender == uiFreqDecaySlider) {
        [uiFreqDecayField setFloatValue:floatValue];
    } else {
        [uiFreqDecaySlider setFloatValue:floatValue];
    }
    
    [self updateVisuals];
}

- (IBAction)iaPartFreqSustainChanged:(id)sender
{
    float floatValue = [sender floatValue];
    
    // check to make sure it's within the range
    if(floatValue < SYRD_PARAM_INFO[kPartFreqSUSLevel].mMinVal)
        floatValue = SYRD_PARAM_INFO[kPartFreqSUSLevel].mMinVal;
    
    if(floatValue > SYRD_PARAM_INFO[kPartFreqSUSLevel].mMaxVal)
        floatValue = SYRD_PARAM_INFO[kPartFreqSUSLevel].mMaxVal;
    
    // determine the parameter number based off which group we're currently focusing on
    int paramNum = kNumGlobalParams + (mTargetPartialGroup * kNumPartParams) + kPartFreqSUSLevel;
    OSStatus result = AUParameterSet(mParameterListener, sender, &mParameter[paramNum], (Float32)floatValue, 0);
    
    NSAssert(result == noErr, @"[SynRand_CocoaView iaPartVolChanged:] AUParameterSet()");
    
    if (sender == uiFreqSustainSlider) {
        [uiFreqSustainField setFloatValue:floatValue];
    } else {
        [uiFreqSustainSlider setFloatValue:floatValue];
    }
    
    [self updateVisuals];
}

- (IBAction)iaPartFreqReleaseChanged:(id)sender
{
    float floatValue = [sender floatValue];
    
    // check to make sure it's within the range
    if(floatValue < SYRD_PARAM_INFO[kPartFreqRELTime].mMinVal)
        floatValue = SYRD_PARAM_INFO[kPartFreqRELTime].mMinVal;
    
    if(floatValue > SYRD_PARAM_INFO[kPartFreqRELTime].mMaxVal)
        floatValue = SYRD_PARAM_INFO[kPartFreqRELTime].mMaxVal;
    
    // determine the parameter number based off which group we're currently focusing on
    int paramNum = kNumGlobalParams + (mTargetPartialGroup * kNumPartParams) + kPartFreqRELTime;
    OSStatus result = AUParameterSet(mParameterListener, sender, &mParameter[paramNum], (Float32)floatValue, 0);
    
    NSAssert(result == noErr, @"[SynRand_CocoaView iaPartVolChanged:] AUParameterSet()");
    
    if (sender == uiFreqReleaseSlider) {
        [uiFreqReleaseField setFloatValue:floatValue];
    } else {
        [uiFreqReleaseSlider setFloatValue:floatValue];
    }
    
    [self updateVisuals];
}

- (IBAction)iaPartFreqScaleChanged:(id)sender
{
    float floatValue = [sender floatValue];
    
    // check to make sure it's within the range
    if(floatValue < SYRD_PARAM_INFO[kPartFreqScale].mMinVal)
        floatValue = SYRD_PARAM_INFO[kPartFreqScale].mMinVal;
    
    if(floatValue > SYRD_PARAM_INFO[kPartFreqScale].mMaxVal)
        floatValue = SYRD_PARAM_INFO[kPartFreqScale].mMaxVal;
    
    // determine the parameter number based off which group we're currently focusing on
    int paramNum = kNumGlobalParams + (mTargetPartialGroup * kNumPartParams) + kPartFreqScale;
    OSStatus result = AUParameterSet(mParameterListener, sender, &mParameter[paramNum], (Float32)floatValue, 0);
    
    NSAssert(result == noErr, @"[SynRand_CocoaView iaPartVolChanged:] AUParameterSet()");
    
    if (sender == uiFreqScaleSlider) {
        [uiFreqScaleField setFloatValue:floatValue];
    } else {
        [uiFreqScaleSlider setFloatValue:floatValue];
    }
}

- (IBAction)iaRandomCentChanged:(id)sender
{
    float floatValue = [sender floatValue];
    
    if (sender == uiRandomSlider) {
        [uiRandomField setFloatValue:floatValue];
    } else {
        [uiRandomSlider setFloatValue:floatValue];
    }
}

- (IBAction)iaRandomAllButtonPushed:(id)sender
{
    float percentage = [uiRandomSlider floatValue] / 50.0f;
    OSStatus result;
    
    for(int i = kNumGlobalParams; i < NUM_PARAMS; i++)
    {
        // determine which partial parameter and group this is
        int hold = i - kNumGlobalParams;

        hold %= kNumPartParams;
        
        bool canToggle = [uiRandomToggleBtn state];
        bool isEnabledBool = hold == kPartEnabledBool;

        // if this parameter is not the enabled bool, or you CAN toggle the enabled bool
        if(!isEnabledBool || canToggle)
        {
            // randomize a number between -1 and 1
            float randomNum = ((random() % 20000000) - 10000000) / 10000000.0f;
            
            // get half of the max range
            float radius = (SYRD_PARAM_INFO[hold].mMaxVal - SYRD_PARAM_INFO[hold].mMinVal) / 2.0f;
            
            // then get a result that is the appropriate percentage of random within the range
            float resultVal = randomNum * percentage * radius;
            
            // get the original value
            float curVal;
            result = AudioUnitGetParameter(mAU, mParameter[i].mParameterID, kAudioUnitScope_Global, 0, &curVal);
            NSAssert(result == noErr, @"[SynRand_CocoaView iaRandomButtonPushed GetParameter] (x.1)");
            
            // add the two
            curVal += resultVal;
            
            // check to make sure it's an integer value if it needs to be
            if((SYRD_PARAM_INFO[hold].mUnitType == kAudioUnitParameterUnit_Boolean) || (SYRD_PARAM_INFO[hold].mUnitType == kAudioUnitParameterUnit_Indexed))
                curVal = (int)(((randomNum + 1)/ 2) + .5f);
            
            // check to make sure it's within the range
            if(curVal < SYRD_PARAM_INFO[hold].mMinVal)
                curVal = SYRD_PARAM_INFO[hold].mMinVal;
            
            if(curVal > SYRD_PARAM_INFO[hold].mMaxVal)
                curVal = SYRD_PARAM_INFO[hold].mMaxVal;
            
            // and assign that value
            result = AUParameterSet(mParameterListener, sender, &mParameter[i], (Float32)curVal, 0);
            NSAssert(result == noErr, @"[SynRand_CocoaView iaRandomButtonPushed SetParameter] (x.1)");
        }
    }
    [self updateVisuals];
}

- (IBAction)iaRandomCurrButtonPushed:(id)sender
{
    float percentage = [uiRandomSlider floatValue] / 50.0f;
    OSStatus result;
    
    for(int i = kNumGlobalParams; i < NUM_PARAMS; i++)
    {
        // determine which partial parameter and group this is
        int hold = i - kNumGlobalParams;
        
        int numGroup = (hold / kNumPartParams);
        hold %= kNumPartParams;
        
        bool canToggle = [uiRandomToggleBtn state];
        bool isEnabledBool = hold == kPartEnabledBool;
        
        if(numGroup != mTargetPartialGroup)
            continue;
        
        // if this parameter is not the enabled bool, or you CAN toggle the enabled bool
        if(!isEnabledBool || canToggle)
        {
            // randomize a number between -1 and 1
            float randomNum = ((random() % 20000000) - 10000000) / 10000000.0f;
            
            // get half of the max range
            float radius = (SYRD_PARAM_INFO[hold].mMaxVal - SYRD_PARAM_INFO[hold].mMinVal) / 2.0f;
            
            // then get a result that is the appropriate percentage of random within the range
            float resultVal = randomNum * percentage * radius;
            
            // get the original value
            float curVal;
            result = AudioUnitGetParameter(mAU, mParameter[i].mParameterID, kAudioUnitScope_Global, 0, &curVal);
            NSAssert(result == noErr, @"[SynRand_CocoaView iaRandomButtonPushed GetParameter] (x.1)");
            
            // add the two
            curVal += resultVal;
            
            // check to make sure it's an integer value if it needs to be
            if((SYRD_PARAM_INFO[hold].mUnitType == kAudioUnitParameterUnit_Boolean) || (SYRD_PARAM_INFO[hold].mUnitType == kAudioUnitParameterUnit_Indexed))
                curVal = (int)(((randomNum + 1)/ 2) + .5f);
               
            // check to make sure it's within the range
            if(curVal < SYRD_PARAM_INFO[hold].mMinVal)
                curVal = SYRD_PARAM_INFO[hold].mMinVal;
            
            if(curVal > SYRD_PARAM_INFO[hold].mMaxVal)
                curVal = SYRD_PARAM_INFO[hold].mMaxVal;
            
            // and assign that value
            result = AUParameterSet(mParameterListener, sender, &mParameter[i], (Float32)curVal, 0);
            NSAssert(result == noErr, @"[SynRand_CocoaView iaRandomButtonPushed SetParameter] (x.1)");
        }
    }
}

#pragma mark ____ NOTIFICATIONS ____
/*
// This routine is called when the user has clicked on the slider. We need to send a begin parameter change gesture to alert hosts that the parameter may be changing value
-(void) handleMouseDown: (NSNotification *) aNotification {
	if ([aNotification object] == uiParam1Slider) {
		AudioUnitEvent event;
		event.mArgument.mParameter = mParameter[0];
		event.mEventType = kAudioUnitEvent_BeginParameterChangeGesture;
		
		AUEventListenerNotify (NULL, self, &event);		// NOTE, if you have an AUEventListenerRef because you are listening to event notification, 
														// pass that as the first argument to AUEventListenerNotify instead of NULL 
	}
}

-(void) handleMouseUp: (NSNotification *) aNotification {
	if ([aNotification object] == uiParam1Slider) {
		AudioUnitEvent event;
		event.mArgument.mParameter = mParameter[0];
		event.mEventType = kAudioUnitEvent_EndParameterChangeGesture;
	
		AUEventListenerNotify (NULL, self, &event);		// NOTE, if you have an AUEventListenerRef because you are listening to event notification, 
														// pass that as the first argument to AUEventListenerNotify instead of NULL 
	}
}
*/

#pragma mark ____ PRIVATE FUNCTIONS ____
- (void)addListeners {
    OSStatus result = AUListenerCreate(	ParameterListenerDispatcher, self, 
                                    CFRunLoopGetCurrent(), kCFRunLoopDefaultMode, 0.100, // 100 ms
                                    &mParameterListener	);
	NSAssert(result == noErr, @"[SynRand_CocoaView _addListeners] AUListenerCreate()");
	
    int i;
    for (i = 0; i < mNumParameters; ++i) {
        mParameter[i].mAudioUnit = mAU;
        result = AUListenerAddParameter (mParameterListener, NULL, &mParameter[i]);
        NSAssert(result == noErr, @"[SynRand_CocoaView _addListeners] AUListenerAddParameter()");
    }
    
   	//[[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(handleMouseDown:) name:SynRand_GestureSliderMouseDownNotification object: uiParam1Slider];
	//[[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(handleMouseUp:) name:SynRand_GestureSliderMouseUpNotification object: uiParam1Slider];
}

- (void)removeListeners {
    OSStatus result;
    int i;
    for (i = 0; i < mNumParameters; ++i) {
        result = AUListenerRemoveParameter(mParameterListener, NULL, &mParameter[i]);
        NSAssert(result == noErr, @"[SynRand_CocoaView _removeListeners] AUListenerRemoveParameter()");
    }
    
    result = AUListenerDispose(mParameterListener);
	NSAssert(result == noErr, @"[SynRand_CocoaView _removeListeners] AUListenerDispose()");
    
    [[NSNotificationCenter defaultCenter] removeObserver: self name:SynRand_GestureSliderMouseDownNotification object: uiGlobalVolSlider];
	[[NSNotificationCenter defaultCenter] removeObserver: self name:SynRand_GestureSliderMouseUpNotification object: uiGlobalVolSlider];
}

- (void)synchronizeUIWithParameterValues {
    OSStatus result;
	Float32 value;
    int i;
    
    for (i = 0; i < mNumParameters; ++i) {
        // only has global parameters
        result = AudioUnitGetParameter(mAU, mParameter[i].mParameterID, kAudioUnitScope_Global, 0, &value);
        NSAssert(result == noErr, @"[SynRand_CocoaView synchronizeUIWithParameterValues] (x.1)");
        
        result = AUParameterSet (mParameterListener, self, &mParameter[i], value, 0);
        NSAssert(result == noErr, @"[SynRand_CocoaView synchronizeUIWithParameterValues] (x.2)");
        
        result = AUParameterListenerNotify (mParameterListener, self, &mParameter[i]);
        NSAssert(result == noErr, @"[SynRand_CocoaView synchronizeUIWithParameterValues] (x.3)");
    }
}

#pragma mark ____ LISTENER CALLBACK DISPATCHEE ____
- (void)parameterListener:(void *)inObject parameter:(const AudioUnitParameter *)inParameter value:(Float32)inValue {
    //inObject ignored in this case.
    
    
    
    if(inParameter->mParameterID < kNumGlobalParams)
    {
        switch (inParameter->mParameterID)
        {
            case kGlobalVolume:
                        [uiGlobalVolSlider setFloatValue:inValue];
                        [uiGlobalVolField setStringValue:[[NSNumber numberWithFloat:inValue] stringValue]];
                        break;
        }
        [self updateVisuals];
    }
    else
    {
        // determine the group
        int group = (inParameter->mParameterID - kNumGlobalParams) / kNumPartParams;
        
        // if this parameter is in the target group, update the sliders to reflect that
        if(group == mTargetPartialGroup)
        {
            // determine the actual parameter number
            const int paramNum = (inParameter->mParameterID - kNumGlobalParams) % kNumPartParams;
            
            // and update accordingly
            switch(paramNum)
            {
                case kPartFreqOffset:
                    [uiPartFreqSlider setFloatValue:inValue];
                    [uiPartFreqField setStringValue:[[NSNumber numberWithFloat:inValue] stringValue]];
                    break;
                case kPartLFOFreq:
                    [uiPartLFOSlider setFloatValue:inValue];
                    [uiPartLFOFreqField setStringValue:[[NSNumber numberWithFloat:inValue] stringValue]];
                    break;
                case kPartVolMod:
                    [uiPartVolSlider setFloatValue:inValue];
                    [uiPartVolField setStringValue:[[NSNumber numberWithFloat:inValue] stringValue]];
                    break;
                case kPartEnabledBool:
                    [uiPartEnabledBtn setState:inValue];
                    break;
                case kPartWaveType:
                    [uiWaveformList selectItemWithTag:(NSInteger)inValue];
                    break;
                case kPartATKTime:
                    [uiAmpAttackSlider setFloatValue:inValue];
                    [uiAmpAttackField setStringValue:[[NSNumber numberWithFloat:inValue] stringValue]];
                    break;
                case kPartDECTime:
                    [uiAmpDecayField setFloatValue:inValue];
                    [uiAmpDecayField setStringValue:[[NSNumber numberWithFloat:inValue] stringValue]];
                    break;
                case kPartSUSLevel:
                    [uiAmpSustainSlider setFloatValue:inValue];
                    [uiAmpSustainField setStringValue:[[NSNumber numberWithFloat:inValue] stringValue]];
                    break;
                case kPartRELTime:
                    [uiAmpReleaseSlider setFloatValue:inValue];
                    [uiAmpReleaseField setStringValue:[[NSNumber numberWithFloat:inValue] stringValue]];
                    break;
                case kPartFreqScale:
                    [uiFreqScaleSlider setFloatValue:inValue];
                    [uiFreqScaleField setStringValue:[[NSNumber numberWithFloat:inValue] stringValue]];
                    break;
                case kPartFreqATKTime:
                    [uiFreqAttackSlider setFloatValue:inValue];
                    [uiFreqAttackField setStringValue:[[NSNumber numberWithFloat:inValue] stringValue]];
                    break;
                case kPartFreqDECTime:
                    [uiFreqDecaySlider setFloatValue:inValue];
                    [uiFreqDecayField setStringValue:[[NSNumber numberWithFloat:inValue] stringValue]];
                    break;
                case kPartFreqSUSLevel:
                    [uiFreqSustainSlider setFloatValue:inValue];
                    [uiFreqSustainField setStringValue:[[NSNumber numberWithFloat:inValue] stringValue]];
                    break;
                case kPartFreqRELTime:
                    [uiFreqReleaseSlider setFloatValue:inValue];
                    [uiFreqReleaseField setStringValue:[[NSNumber numberWithFloat:inValue] stringValue]];
                    break;
            }
            
            [self updateVisuals];
        }
        
    }
	
}

- (void)updateVisuals
{
    if(uiPartEnabledBtn.state)
    {
        [uiPartNumLabel setTextColor:[NSColor greenColor]];
    }
    else
    {
        [uiPartNumLabel setTextColor:[NSColor redColor]];
    }
    [uiAmpADSRView updateValues:[uiAmpAttackSlider floatValue] dec:[uiAmpDecaySlider floatValue] sus:[uiAmpSustainSlider floatValue] rel:[uiAmpReleaseSlider floatValue]];
    [uiFreqADSRView updateValues:[uiFreqAttackSlider floatValue] dec:[uiFreqDecaySlider floatValue] sus:[uiFreqSustainSlider floatValue] rel:[uiFreqReleaseSlider floatValue]];
    [uiAmpADSRView setNeedsDisplay:true];
    [uiFreqADSRView setNeedsDisplay:true];
}

@end
