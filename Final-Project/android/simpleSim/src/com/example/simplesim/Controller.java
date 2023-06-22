package com.example.simplesim;

import android.view.View;

public class Controller {
	public native int gearUp(int curr);
	public native int gearDown(int curr);
	
	
	public static int gearChange(Automobile car, View v, int value, int dir){
		int pos = value + dir;
		car.setPos(Automobile.GearPos.values()[pos]);
		return pos;
	}
	
}
