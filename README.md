# TEVEL-2 Flight Software - Ido and Daniel
This repository contains the flight software for the TEVEL-2 program CubeSat developed by Ido Abelman and Daniel Sorin for their final project, based on templates and code developed by Tel Aviv University in collaboration with ISIS - Innovative Solutions In Space.

The flight software is written in C and uses freeRTOS, an open-source real-time operating system. The software is designed to control and manage the CubeSat's onboard subsystems, including communication, power, and data storage.

The following repository contains:
1. Our implementation of the flight software under "GlobusSatProj".
2. ISIS headers and demo code for the hardware abstraction layers (hal) and the satellite subsystem libraries under "hal" and "satellite-subsystems".

## Getting Started
To get started with the TEVEL-2 Flight Software, please follow these steps:

• Install ISIS-SDK from [here](https://drive.google.com/file/d/0B0gJIJIi4GEBcV91cWlNTTQ3Tnc/view?usp=drive_link&resourcekey=0-4HGLfnsb0zMTF5DQNIoXMg)
After installing, the IDE for the project should be in ```"C:\ISIS\application\eclipse\eclipse.exe"```, unless you specified otherwise 

• Set a local copy of this GitHub repository. Do so by [forking and cloning the repository](https://docs.github.com/en/get-started/quickstart/fork-a-repo) or [cloning the repository](https://docs.github.com/en/github/creating-cloning-and-archiving-repositories/cloning-a-repository) using GitBash and 
```
cd C:\ISIS\workspace
git clone https://github.com/adiwwww/GlobusSatProject2.git
```

• Set up your environment as seen in [ISIS quickstart guide](https://drive.google.com/file/d/1y2gOld5oa4XrHUUzJRc_xc5E65OoDaRe/view?usp=drive_link)

• The software has two main compilation modes - testing mode and regular mode, which can be set using the preprocessor flag "TESTING" in TestingConfigurations.h. In addition to those modes either "GOMEPS" or "ISISEPS" can be set to define which EPS system we are using.

• The software can be uploaded to the satellite and run also as explained in the quickstart guide.

## File Structure
Our code is under GlobusSatProject/src and includes:

    │   FRAM_FlightParameters.h             # locations and default values of FRAM paramteres
    │   GlobalStandards.h                   # definitions used across the project
    │   initSystem.c                        # initialization implementation
    │   InitSystem.h                        
    │   main.c                              # main entry point
    │   main.h                              
    │   StateMachine.c                      # state machine implementation
    │   StateMachine.h
    │   SysI2CAddr.h                        # I2C addresses of subsystems
    │   TestingConfigurations.h             # Config defines
    │   utils.c                             # some util functions
    │   utils.h
    │
    ├───SubSystemModules                    # Modules managing the various subsystems
    │   ├───Communication                   # Management of TRXVU and command handling
    │   │   │   CommandDictionary.c         # implementation of the subsystem command routers
    │   │   │   CommandDictionary.h
    │   │   │   SatCommandHandler.c         # implementation of command parsing and the main command router
    │   │   │   SatCommandHandler.h
    │   │   │   SPL.h                       # definitions for the SPL protocol
    │   │   │   TRXVU.c                     # main TRXVU management implementation
    │   │   │   TRXVU.h
    │   │   │
    │   │   └───SubsystemCommands           # Files containing all the functions implementing commands          
    │   │           EPS_Commands.h
    │   │           Management_Commands.h
    │   │           Telemetry_Commands.h
    │   │           TRXVU_Commands.c
    │   │           TRXVU_Commands.h
    │   │
    │   ├───Housekeeping                    # Implementation of telemetry collection and the filesystem 
    │   │       TelemetryCollector.c        # telemetry collection implementation
    │   │       TelemetryCollector.h
    │   │       TelemetryFiles.h            # defines for the telemetry files
    │   │       TLM_management.c            # management of the telemetry files
    │   │       TLM_management.h
    │   │
    │   ├───Maintenance                     # Maintenance submodule implementation
    │   │       Maintenance.c
    │   │       Maintenance.h
    │   │
    │   └───PowerManagement
    │           EPS.c
    │           EPS.h
    │
    └───TestingDemos                        # Test files (will run starting in "MainTest.c" if "TESTING" is set)
            CommandsTestingDemo.h
            EpsStub.c
            EpsStub.h
            EpsTestingDemo.c
            EpsTestingDemo.h
            FileSystemTestingDemo.h
            MaintenanceTestingDemo.h
            MainTest.c
            MainTest.h
            TelemetryTestingDemo.c
            TelemetryTestingDemo.h
            TrxvuTestingDemo.c
            TrxvuTestingDemo.h
