package com.example.simplesim;

import android.view.View;

public class Controller {
	public native int gearUp(int curr);
	public native int gearDown(int curr);
	
	public static int ignite(Automobile car, boolean mode) {
		if(mode == true) {
			car.setEngineStat(true);
		}
		else {
			
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
	
}
