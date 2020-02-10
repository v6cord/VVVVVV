/* vi: set ft=objc */

#import <IOKit/ps/IOPowerSources.h>
#import <Foundation/Foundation.h>

int battery_level() {
    int percentage = 100;

    CFTypeRef powerSourceInfo = IOPSCopyPowerSourcesInfo();
    CFArrayRef powerSources = IOPSCopyPowerSourcesList(powerSourceInfo);

    long numberOfSources = CFArrayGetCount(powerSources);
    if(numberOfSources != 0) {
        CFDictionaryRef powerSource = IOPSGetPowerSourceDescription(powerSourceInfo, CFArrayGetValueAtIndex(powerSources, 0));

        const void *psValue;
        int curCapacity = 0;
        int maxCapacity = 0;

        psValue = CFDictionaryGetValue(powerSource, @kIOPSCurrentCapacityKey);
        CFNumberGetValue((CFNumberRef)psValue, kCFNumberSInt32Type, &curCapacity);

        psValue = CFDictionaryGetValue(powerSource, @kIOPSMaxCapacityKey);
        CFNumberGetValue((CFNumberRef)psValue, kCFNumberSInt32Type, &maxCapacity);

        percentage = (int)((double)curCapacity/(double)maxCapacity * 100);
    }

    CFRelease(powerSourceInfo);
    CFRelease(powerSources);

    return percentage;
}

_Bool on_battery() {
    _Bool onBattery = 0;

    CFTypeRef powerSourceInfo = IOPSCopyPowerSourcesInfo();
    CFArrayRef powerSources = IOPSCopyPowerSourcesList(powerSourceInfo);

    CFDictionaryRef powerSource = NULL;

    long numberOfSources = CFArrayGetCount(powerSources);
    if(numberOfSources != 0) {
        powerSource = IOPSGetPowerSourceDescription(powerSourceInfo, CFArrayGetValueAtIndex(powerSources, 0));

        NSString str = CFDictionaryGetValue(powerSource, @kIOPSPowerSourceStateKey);

        if ([str isEqualToString:@kIOPSBatteryPowerValue]) {
            onBattery = 1;
        }
    }

    CFRelease(powerSourceInfo);
    CFRelease(powerSources);

    return onBattery;
}
