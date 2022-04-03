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
        SetFrontProcess(&psn);

        [app setDelegate:delegate];
        [app run];

        NSLog(@"End");
}
