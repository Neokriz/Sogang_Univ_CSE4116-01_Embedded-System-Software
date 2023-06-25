package com.example.simplesim;

import android.os.Handler;
import android.os.Message;
import android.util.Log;

public class InterruptDetector extends Thread {
	
	int retValue = 0;
//	Handler iHandler;
//	
//	InterruptDetector(Handler handler) {
//		iHandler = handler;
//	}
	public void run(int fd) {
		while(true) {
			Message msg = Message.obtain();
			msg.what = 0;
			msg.arg1 = retValue;
			//iHandler.sendMessage(msg);
			
			retValue = DeviceController.readInterrupt(fd, "0000");
			Log.d("call readInterrupt", ""+retValue);
			try{sleep(Long.MAX_VALUE);}
			catch(InterruptedException e) {
				Log.d("InterruptedException", "");
			}
		}
	}

}
