#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

#import "app_delegate.h"

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
