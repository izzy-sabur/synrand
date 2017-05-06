//
//  ADSRView.m
//  SynRand
//
//  Created by Poppy on 4/20/17.
//  Copyright (c) 2017 Penguin. All rights reserved.
//

#import "ADSRView.h"

@implementation ADSRView

- (void)updateValues:(float)atk dec:(float)dec sus:(float)sus rel:(float)rel
{
    m_atk = atk;
    m_dec = dec;
    m_sus = sus;
    m_rel = rel;
    m_timeRatio = [self bounds].size.width / ((m_atk + m_dec + m_rel) * 2);
    m_susTime = (m_atk + m_dec + m_rel);
}

- (void)drawRect:(NSRect)dirtyRect
{
    NSBezierPath* envPath = [NSBezierPath new];
    [[NSColor blackColor] set];
    [envPath moveToPoint:NSMakePoint(0, 0)];
    [envPath lineToPoint:NSMakePoint(m_atk * m_timeRatio, [self bounds].size.height)];
    [envPath lineToPoint:NSMakePoint((m_atk + m_dec) * m_timeRatio, m_sus * [self bounds].size.height)];
    [envPath lineToPoint:NSMakePoint((m_atk + m_dec + m_susTime) * m_timeRatio, m_sus * [self bounds].size.height)];
    [envPath lineToPoint:NSMakePoint([self bounds].size.width, 1)];
    [envPath fill];
    // Drawing code here.
    
}

@end
