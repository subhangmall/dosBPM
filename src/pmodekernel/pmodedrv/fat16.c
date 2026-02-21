#include <stdint.h>

#pragma pack(push, 1)
struct BootSector {
    uint8_t jmpToBootRoutine[3];
    uint8_t manufacturerNameAndVersionNumber[8];
    uint8_t bytesPerSector[2];
    uint8_t sectorsPerCluster;
    uint8_t numReservedSectors[2];
    uint8_t numFATs;
    uint8_t numEntriesInRootDir[2];
    uint8_t numSectorsInVolume[2];
    uint8_t mediaDesc;
    uint8_t numSectorsPerFAT[2];
    uint8_t numsectorsPerTrack[2];
    uint8_t numRWHeads[2];
    uint8_t numHiddenSectors[2];
    uint8_t bootRoutine[482];
}
#pragma pack(pop)


void initHDDType() {
    if () {

    }
}