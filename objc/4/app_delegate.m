#import "app_delegate.h"
#import "view.h"
#import "window.h"

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
        NSLog(@"applicationDidFinishLaunching");

        NSRect rect = NSMakeRect(0, 0, 500, 500);
        /*
         * Note: a window with NSWindowStyleMaskBorderless
         * can't be main or key by default. As we want to make our view
         * receive key events, MyWindow has canBecomeKeyWindow and
         * canBecomeMainWindow overridden.
         *
         * https://developer.apple.com/documentation/appkit/nswindowstylemask/nswindowstylemaskborderless
         */
        NSUInteger style = NSWindowStyleMaskBorderless;
        NSWindow *window = [MyWindow alloc];
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

        NSView *view = [MyView alloc];
        [window.contentView addSubview:view];
        [window makeFirstResponder:view];
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
