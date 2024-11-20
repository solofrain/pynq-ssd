#!../../bin/linux-arm/zDDM

## You may have to change zDDM to something else
## everywhere it appears in this file

< envPaths

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/zDDM.dbd"
zDDM_registerRecordDeviceDriver pdbbase
#devAoI2CConfig(2,1,8)
#devAiI2CConfig(0,1,8)
#devAiI2CConfig(2,1,8)
#devSPIConfig(0,1,8)
#devSPI2Config(0,1,8)
devzDDMConfig(1,1,384)


## Load record instances
#dbLoadTemplate "db/user.substitutions"
dbLoadRecords "db/det1.db"
#dbLoadRecords "db/i2cDacs.db"
#dbLoadRecords "db/i2cAdcs.db"
#dbLoadRecords "db/SpiDacs.db","user=det1"
#dbLoadRecords "db/spi2Adcs.db"
#dbLoadRecords "db/images.db"


## Set this to see messages from mySub
#var mySubDebug 1

## Run this to trace the stages of iocInit
#traceIocInit
var devI2CDebug 21 
var zDDMRecordDebug 0
var devzDDMdebug 0

#system "/root/maia/maia_zserver12 &"

cd "${TOP}/iocBoot/${IOC}"
iocInit

#Enable Scepter
#poke 7 1
#enable PDD readout
#poke 8 0xfffff
#
## Start any sequence programs
#seq sncExample, "user=peter"
