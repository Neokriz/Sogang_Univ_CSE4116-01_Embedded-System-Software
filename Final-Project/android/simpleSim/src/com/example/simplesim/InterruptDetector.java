package com.example.simplesim;

import android.os.Handler;
import android.os.Message;

public class InterruptDetector extends Thread {
	
	int retValue = 0;
	Handler iHandler;
	
	InterruptDetector(Handler handler) {
		iHandler = handler;
	}
	public void run(int fd) {
		while(true) {
			Message msg = Message.obtain();
			msg.what = 0;
			msg.arg1 = retValue;
			iHandler.sendMessage(msg);
			DeviceController.readInterrupt(fd, "0000");
		}
	}

}
