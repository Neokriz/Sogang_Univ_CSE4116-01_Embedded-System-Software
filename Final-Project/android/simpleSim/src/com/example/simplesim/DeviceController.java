package com.example.simplesim;

public class DeviceController {
	public native int openSim();
	public native int closeSim(int fd);
	public native int writeToDevice(int fd, String data);
	public native int ioctlSetSim(int fd, String[] message);
	public native int ioctlCmdSim(int fd, String message);
	
	public native int openSimInt();
	public native int closeSimInt(int fd);
	public native int readInterrupt(int fd, String data);
}
