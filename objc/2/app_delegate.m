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

        NSRect buttonFrame = NSMakeRect(20, 20, 100, 100);
        NSButton *button = [NSButton alloc];
        [button initWithFrame:buttonFrame];
        [button setButtonType:NSButtonTypeMomentaryLight];
        [button setTitle:@"Hello"];
        [button setTarget:self];
        [button setAction:@selector(myButtonAction:)];
        [window.contentView addSubview:button];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)app
{
        NSLog(@"applicationShouldTerminateAfterLastWindowClosed");
        return YES;
}

- (void)myButtonAction:(NSButton *)button
{
        NSLog(@"myButtonAction");
}

@end
