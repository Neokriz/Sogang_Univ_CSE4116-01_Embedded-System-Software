package com.example.simplesim;

import java.util.Random;

import android.util.Log;

public class Automobile {
	private boolean engineStat;
    private int rpm;
    private int gear;
    private GearPos pos;
    private double speed;
    
	public enum GearPos {
		P("Parking"),
		R("Reverse"),
		N("Neutral"),
		D("Drive"),
		M("Manual")
		;
		private final String label;
		
		GearPos(String label){
			this.label = label;
		}
		
		public String label() {
			return label;
		}
	}

    public Automobile() {
    	this.engineStat = false;
        this.rpm = 0;
        this.gear = 0;
        this.pos = GearPos.P;
        this.speed = calculateSpeed();
    }
    
    public boolean getEngineStat(){
    	return engineStat;
    }
    
    public void setEngineStat(boolean engineStat){
    	this.engineStat = engineStat;
        Log.d("EngineStatus", ""+this.engineStat);
    }

    public int getRpm() {
        return rpm;
    }

    public void setRpm(int rpm) {
        this.rpm = rpm;
        this.speed = calculateSpeed();
    }
    
    public int getGear() {
        return gear;
    }

    public void setGear(int gear) {
        this.gear = gear;
        this.speed = calculateSpeed();
    }

    public GearPos getPos() {
        return pos;
    }

    public void setPos(GearPos pos) {
        this.pos = pos;
        switch(pos) {
        case P:
        	setGear(0);
        	break;
        case R:
        	setGear(-1);
        	break;
        case N:
        	setGear(0);
        	break;
        case D:
        	setGear(1);
        	break;
        }
    }

    public double getSpeed() {
        return speed;
    }

    private double calculateSpeed() {
        // Speed calculation logic based on RPM and gear
        double speed = rpm * pos.ordinal() * 0.01; // TODO: Adjust the calculation formula according to your requirements
        return speed;
    }
	
    public void updateRpm(int rpm) {
        Random random = new Random();
        this.rpm = random.nextInt(21) + 970 + rpm; // Generate a random number between 970 and 990
        //Log.d("UmpdateRPM", ""+rpm);
    }
}
