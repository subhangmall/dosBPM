#include <stdint.h>

#pragma pack(push, 1)

// NEED TO REVERSE ALL OF THIS BECAUSE ACCIDENTALLY INVERTED IT 

// 3F0
struct StatusRegisterA {
    uint8_t interruptPending : 1;
    uint8_t secondDriveInstalled : 1;
    uint8_t step : 1;
    uint8_t track0 : 1;
    uint8_t head1Select : 1;
    uint8_t idx : 1;
    uint8_t writeProtect : 1;
    uint8_t direction : 1;
};

// 3F1
struct StatusRegisterB {
    uint8_t reserved : 2;
    uint8_t driveSelect : 1;
    uint8_t writeData : 1;
    uint8_t readData : 1;
    uint8_t writeEnable : 1;
    uint8_t motorEnable1: 1;
    uint8_t motorEnable0 : 1;
};

// WRITE ONLY
// 3F2
struct DigitalOutputByte {
    uint8_t motorDEnabled : 1;
    uint8_t motorCEnabled : 1;
    uint8_t motorBEnabled : 1;
    uint8_t motorAEnabled : 1;
    uint8_t dmaAndIRQChannelEnabled : 1;
    uint8_t controllerEnabledOrShouldreset : 1;
    uint8_t driveSelectNumber : 2;
};

// READ ONLY
// 3F4
struct MainStatusByte {
    uint8_t dataRegisterReady : 1;
    uint8_t DIO : 1; // REVIEW
    uint8_t nonDMAModeEnabled : 1;
    uint8_t deviceIsBusy : 1;
    uint8_t driveDInPositioningMode : 1;
    uint8_t driveCInPositioningMode : 1;
    uint8_t driveBInPositioningMode : 1;
    uint8_t driveAInPositioningMode : 1;
};

// 3F5
struct CommandStatusRegister0 {
    uint8_t lastCommandStatus : 2;
    uint8_t fddCompletedSeekCommand : 1;
    uint8_t equipmentCheck : 1;
    uint8_t notReadyForReadWriteOrSSAccessToHead1 : 1;
    uint8_t headNumAtInterrupt : 1; // 0 or 1
    uint8_t unitSelectedAtInterrupt : 2; // 0 = a, 1 = b, 2 = c, 3 = d
};

// 3F5
// no need to reverse, already fine
struct CommandStatusRegister1 {
    uint8_t cannotFindIDAddrMark : 1;
    uint8_t writeProtectDetectedDuringWrite : 1;
    uint8_t cannotFindSectorId : 1;
    uint8_t unusedBit : 1; // literally just ignore this
    uint8_t controllerNotServicedInReasonableTime : 1; // overran
    uint8_t dataErrorInIDOrDataField : 1;
    uint8_t unusedBit2 : 1;
    uint8_t endOfCylinderReached : 1;
};

// 3F5
// no need to reverse, already fine
struct CommandStatusRegister2 {
    uint8_t missingAddressMarkInDataField : 1;
    uint8_t badCylinder : 1;
    uint8_t sectorNotFoundInCylinder : 1;
    uint8_t scanCmdEqualConditionSatisfied : 1;
    uint8_t wrongCylinderDetected : 1;
    uint8_t crcErrInSectorData : 1;
    uint8_t sectorWithDeletedDataAddressMarkDetected : 1;
    uint8_t unusedBit : 1;
};

// 3F5
// no need to reverse, already fine
struct CommandStatusRegister3 {
    uint8_t fddUnitSelected : 2; // 0 = a, 1=b, etc
    uint8_t fddSideHeadSelectStatus : 1; // 0 = head 0, 1 = head 1
    uint8_t fddTwoSidedStatusSignal : 1;
    uint8_t fddTrackZeroStatusSignal : 1;
    uint8_t fddReadyStatusSignal : 1;
    uint8_t fddWriteProtectStatusSignal : 1;
    uint8_t fddFaultStatusSignal : 1;
};

// 3F5
struct DataRegisterByte {
    uint8_t interruptCode : 2; 
    /*
        00 -> normal termination, finished
        01 -> the controller couldn't finish the command
        10 -> invalid command
        11 -> drive became not ready
    */
    uint8_t hasCompletedASeekOrCalibrartionCmd : 1; // can also be when you executed a r/w command which has an implicit seek
    uint8_t unitCheck : 1; // see if drive faults or if recalibrate can't find track 0 after 79 pulses
    uint8_t driveNotReady : 1;
    uint8_t headCurrentlyActive : 1; // 1 = head 1, 0 = head 0
    uint8_t currentlySelectedDrive : 2; // 00 = drive 0 (A:\), 01 = drive 1 (B:\), etc
};

// 3F7
// dont need to reverse
struct DigitalInputRegister {
    uint8_t highDensitySelect : 1;
    uint8_t reserved : 6;
    uint8_t disketteChange : 1;
};

// 3F7
// dont need to reverse
struct ConfigControlRegister {
    uint8_t drc1OrZeroMode : 2; 
    /*
    00: 500000 bit/sec mode
    01: reserved
    10: 250000 bit/sec mode
    11: reserved
    */
    uint8_t reserved : 6;
};

#pragma pack(pop)

/*
HD -> head number selected
MT -> multitrack
MF -> MFM mode
ND -> non-DMA mode
SK -> skip deleted data address mark
US0 -> drive select bit 0
US1 -> drive select bit 1;
*/

