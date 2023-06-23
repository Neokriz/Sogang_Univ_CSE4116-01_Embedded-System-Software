package com.example.simplesim;

import android.util.Log;

import com.example.simplesim.Automobile.GearPos;
import com.example.simplesim.ShiftPatternTable;

public class Controller {
	public native int gearUp(int curr);
	public native int gearDown(int curr);
	
	private static int ignitionProcess; // 0 is defualt(Engine OFF), 1 is ignition process, -1 is shutdown process.
	private static int acceleratation;	// 0 is default(throttle off), 1 is pressing throttle, -1 is throttle off.
	
    public Controller() {
    	Controller.ignitionProcess = 0;
    	Controller.acceleratation = 0;
    }
	
	public int getIgnitionprocess() {
		return ignitionProcess;
	}
	
	public void setIgnitionprocess(int value) {
		Controller.ignitionProcess = value;
	}
	
	public int getAcceleratation() {
		return acceleratation;
	}
	
	public void setAcceleratation(int value) {
		Controller.acceleratation = value;
	}
	
	public static int ignite(Automobile car, boolean mode) {
		if(mode == true) {
			car.setEngineStat(true);
		}
		else {
			car.setEngineStat(false);
		}
		return 0;
	}
	
	public static int engineStart(Automobile car) {
		car.setRpm(car.getRpm()+23);
		if(car.getRpm() >= 970) {
			ignitionProcess = 0;
			Log.d("IgnitionProcess", ""+ignitionProcess);
		}
		return ignitionProcess;
	}
	
	public static int engineShutdown(Automobile car) {
		car.setRpm(car.getRpm()-23);
		if(car.getRpm() <= 40) {
			ignitionProcess = 0;
			car.setRpm(0);
		}
		return ignitionProcess;
	}
	
	public static int idle(Automobile car){
		if(car.getEngineStat()) {
			car.updateRpm(0);
		}
		else {
			car.setRpm(0);
		}
		return 0;
	}
	
	public static int gearChange(Automobile car, int value, int dir){
		int pos = value + dir;
		car.setPos(Automobile.GearPos.values()[pos]);
		return pos;
	}
	
	public static int shiftUp(Automobile car) {
		int currGear;

		currGear = car.getGear();
		if(currGear > 0 && currGear <= 7) {
			car.setGear(currGear+1);
		}
		return car.getGear();
	}
	
	public static int shiftDown(Automobile car) {
		int currGear;

		currGear = car.getGear();
		if(currGear > 1 && currGear <= 8) {
			car.setGear(currGear-1);
		}
		return car.getGear();
	}
	
	public static int accelerate(Automobile car, int input, int throttle) { //input is +1 or -1
		//Log.d("accelerate called", ""+throttle);
		int gwsPosition = car.getPos().ordinal();
		double increment;
		double ratio =  Automobile.GearRatio.values()[car.getGear()+1].getValue();
		if(input > 0) {
			increment = 3 * input * ((double)throttle / 100) * ratio;
			increment = (increment > 1) ? increment : 0;
			Log.d("increment", ""+increment);
		}
		else {
			increment = 5 * input;
		}
		
		if(car.getRpm() < 770) { //TODO:FIX need
			acceleratation = 0;
		}
		
		if(gwsPosition > Automobile.GearPos.P.ordinal()) {
			//increase RPM
			car.setRpm(car.getRpm()+(int)increment);
			if(gwsPosition == Automobile.GearPos.D.ordinal()) {
				boolean accel = (input > 0) ? true : false;
				driveMode(car, throttle, accel);
			}
		}
			
		return 1;
	}
	
	public static void driveMode(Automobile car, int throttle, boolean accel) {
		//Log.d("driveMode, throttle:", ""+throttle);
		int row = throttle / 10;
		int column = 0;  //gear value is between 1 to 8;
		double weightVal = (throttle % 10) * 0.1;
		int carGear = car.getGear();
		int carRpm = car.getRpm();
		int shiftRpm = 9999;
		int plusRpm = 0;

		//accel == true, up shift
		if(accel == true && carGear < 8) {
			column = carGear - 1;
			shiftRpm = ShiftPatternTable.getUpValue(row, column);
			
			//Log.d("row", ""+row);
			//Log.d("column", ""+column);
			//Log.d("weightVal", ""+weightVal);
			//Log.d("carGear", ""+carGear);
			//Log.d("shiftRpm", ""+shiftRpm);
						
			if(weightVal != 0) {
				plusRpm = (int)(weightVal * (ShiftPatternTable.getUpValue(row + 1, column) - shiftRpm));
			}
			if(carRpm >= (shiftRpm + plusRpm)) {
				car.setGear(carGear + 1);
				car.updateRpm(car.getSpeed());
			}
		}
		//accel == false, down shift
		else if(carGear > 1){
			row = 10;
			column = car.getGear() - 2;
			shiftRpm = ShiftPatternTable.getDownValue(row, column);

			if(weightVal != 0) {
				//plusRpm = (int)(weightVal * (ShiftPatternTable.getDownValue(row + 1, column) - shiftRpm));
			}
			if(carRpm <= (shiftRpm + plusRpm)) {
				car.setGear(carGear - 1);
				car.updateRpm(car.getSpeed());
			}
		}
	}
}