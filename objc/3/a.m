#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

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

@end

int
main(int argc, const char **argv)
{
        NSLog(@"Start");

        NSApplication *app = [NSApplication sharedApplication];
        AppDelegate *delegate = [AppDelegate alloc];

        [app setDelegate:delegate];
        [app run];

        NSLog(@"End");
}
