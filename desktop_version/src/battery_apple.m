/* vi: set ft=objc */

#import <IOKit/ps/IOPowerSources.h>
#import <Foundation/Foundation.h>

int battery_level() {
    CFTypeRef powerSourceInfo = IOPSCopyPowerSourcesInfo();
    CFArrayRef powerSources = IOPSCopyPowerSourcesList(powerSourceInfo);

    CFDictionaryRef powerSource = NULL;

    long numberOfSources = CFArrayGetCount(powerSources);
    if(numberOfSources == 0) {
        return 100;
    } else {
        powerSource = IOPSGetPowerSourceDescription(powerSourceInfo, CFArrayGetValueAtIndex(powerSources, 0));

        const void *psValue;
        int curCapacity = 0;
        int maxCapacity = 0;
        int percentage;

        psValue = CFDictionaryGetValue(powerSource, CFSTR(kIOPSCurrentCapacityKey));
        CFNumberGetValue((CFNumberRef)psValue, kCFNumberSInt32Type, &curCapacity);

        psValue = CFDictionaryGetValue(powerSource, CFSTR(kIOPSMaxCapacityKey));
        CFNumberGetValue((CFNumberRef)psValue, kCFNumberSInt32Type, &maxCapacity);

        percentage = (int)((double)curCapacity/(double)maxCapacity * 100);

        return percentage;
    }
}

_Bool on_battery() {
    CFTypeRef powerSourceInfo = IOPSCopyPowerSourcesInfo();
    CFArrayRef powerSources = IOPSCopyPowerSourcesList(powerSourceInfo);

    CFDictionaryRef powerSource = NULL;

    long numberOfSources = CFArrayGetCount(powerSources);
    if(numberOfSources == 0) {
        return 0;
    } else {
        powerSource = IOPSGetPowerSourceDescription(powerSourceInfo, CFArrayGetValueAtIndex(powerSources, 0));

        NSString str = CFDictionaryGetValue(powerSource, CFSTR(kIOPSPowerSourceStateKey));

        if ([str isEqualToString:@kIOPSBatteryPowerValue]) {
            return 1;
        } else {
            return 0;
        }
    }
}
