# CanSscLinux- This Readme is more a compilation of rambling notes than a user manual, I hope this is as usefull as I think it will be ;-)

Linux implementation of CanSsc.  This code could be used for Payload or Host, change the #define CAN_SSC_NODE_ID 0 found in config.h accordingly. Node 0 is the Host.

This system lacks a scheduler so it is passive on the bus.  The application is command-line driven (menu text provides the options) and provides a breakdown of the low-level message breakdown on the system.  This must be modified to reduce debug loading on a real system.  We are far from a final system though and fine detail of the comms will help with Payload integration.

The executable links the Ssc can bus with files in the data folder for each message.  To mimic the operation of another system in the OBC delivering data, there is a second app titled SpoofData.  This app updates an ASCII timestamp regularly and provides an easy way to adjust the data files.  The data files are stored as .txt despite the CanSscLinux app decoding the data as binary.

The script setupCanDriver.sh configures a SocketCan device, this will need to be done each time the PC is booted / the can device is reconnected.  There are ways to automate this and I expect the Gom space environment will already have this sorted out.
To run the script type  ./setupCanDriver.sh from a terminal in the CanSscLinux directory.

There are many Linux to Can adaptors, we have access to a Microchip Can Bus Analyzer (https://www.microchip.com/en-us/development-tool/apgdt002) which has been quite good but I have found that it can get stuck in listen-only mode.  I used DSD TECH USB to CAN Bus Adapter Based on Open Hardware Canable (https://www.amazon.co.uk/dp/B0BQ5G3KLR?ref=ppx_yo2ov_dt_b_fed_asin_title) which seemed more reliable.  Neither of these are isolated and I recommend purchasing an isolated unit from a reliable source for connecting to the Satellite.
Have alternative options available! That will make it easier to find out where the fault in the system is.  If you only have one converter you can easily waste time messing about with kit.

Follow this link to find out more about can bus access from the terminal: https://blog.mbedded.ninja/programming/operating-systems/linux/how-to-use-socketcan-with-the-command-line-in-linux/
Another tool in the box to help decipher faults in the system!

Connection and termination.
Although the ground often isn't required the GOM Space manual clearly states that it must be used.
Some cheapo laptop power supplies can wipe out the CAN bus comms, if you are having issues get a PicoScope on the lines (remember to check both high and low)
Some drivers are ok with limited termination, while others (like the Microchip Can Bus Analyzer) want to see 60ohm before operating properly.  set up the bus as a daisy chain with 120ohm at each end and it should be all good.





