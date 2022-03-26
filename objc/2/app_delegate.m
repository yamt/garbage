#import "app_delegate.h"

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
        NSLog(@"applicationDidFinishLaunching");

        NSRect rect = NSMakeRect(0, 0, 500, 500);
        NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable;
        NSWindow *window = [NSWindow alloc];
        [window initWithContentRect:rect
                          styleMask:style
                            backing:NSBackingStoreBuffered
                              defer:false];
        [window setTitle:@"Hello"];
        [window makeKeyAndOrderFront:self];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app
{
        NSLog(@"applicationShouldTerminateAfterLastWindowClosed");
        return YES;
}

@end
