#import "window.h"

@implementation MyWindow
- (BOOL)canBecomeKeyWindow
{
        NSLog(@"canBecomeKeyWindow");
        return YES;
}
- (BOOL)canBecomeMainWindow
{
        NSLog(@"canBecomeMainWindow");
        return YES;
}
@end
