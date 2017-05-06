//
//  ADSRView.h
//  SynRand
//
//  Created by Poppy on 4/20/17.
//  Copyright (c) 2017 Penguin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface ADSRView : NSView
{
    float m_atk, m_dec, m_sus, m_rel;
    float m_timeRatio, m_susTime;
}
- (void)updateValues:(float)atk dec:(float)dec sus:(float)sus rel:(float)rel;
@end
