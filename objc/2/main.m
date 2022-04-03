#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

#import "app_delegate.h"

int
main(int argc, const char **argv)
{
        NSLog(@"Start");

        NSApplication *app = [NSApplication sharedApplication];
        AppDelegate *delegate = [AppDelegate alloc];

        ProcessSerialNumber psn = {0, kCurrentProcess};
        TransformProcessType(&psn, kProcessTransformToForegroundApplication);

        [app setDelegate:delegate];
        [app activateIgnoringOtherApps:YES];
        [app run];

        NSLog(@"End");
}
