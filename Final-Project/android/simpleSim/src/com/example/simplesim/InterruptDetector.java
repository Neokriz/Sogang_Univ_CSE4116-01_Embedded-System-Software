package com.example.simplesim;

import android.util.Log;

public class InterruptDetector extends Thread {
	
	int retValue = 0;

	public void run(int fd) {
		
    	Log.d("The thread is working.", "Yaaaaa");

		while(true) {
			//retValue = DeviceController.readInterrupt(fd, "0000");
			try{
				Thread.sleep(1000);
				Log.d("call readInterrupt", "calllllllllllllllllllllllllllll"+retValue);
			}
			catch(InterruptedException e) {
				e.printStackTrace();
				Log.d("InterruptedException", "");
			}
		}
	}

}
