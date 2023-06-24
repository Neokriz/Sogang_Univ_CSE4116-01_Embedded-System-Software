package com.example.simplesim;

public class DeviceController {
	public native int openSim();
	public native int ioctlSetSim(int fd, String[] message);
	public native int writeToDevice(int fd, String data);
	
	
	
}
