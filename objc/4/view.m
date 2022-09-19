#import "view.h"

/*
 * https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/EventOverview/Introduction/Introduction.html#//apple_ref/doc/uid/10000060i-CH1-SW1
 * https://boredzo.org/blog/archives/2007-05-22/virtual-key-codes
 * https://eastmanreference.com/complete-list-of-applescript-key-codes
 *
 * It seems that keycode constants like kVK_ANSI_A are only provided
 * for Carbon.
 */

@implementation MyView

- (void)keyDown:(NSEvent *)event
{
        NSLog(@"keyDown %u %@ %u", (unsigned int)event.type, event.characters,
              event.keyCode);
}
- (void)keyUp:(NSEvent *)event
{
        NSLog(@"keyUp   %u %@ %u", (unsigned int)event.type, event.characters,
              event.keyCode);
}
- (BOOL)acceptsFirstResponder
{
        NSLog(@"acceptsFirstResponder");
        return YES;
}
- (BOOL)becomeFirstResponder
{
        NSLog(@"becomeFirstResponder");
        return YES;
}
@end
