package com.example.simplesim;

import java.util.Random;

import android.util.Log;

public class Automobile {
	private boolean engineStat;
    private int rpm;
    private int gear;
    private GearPos pos;
    private double speed;
    
    private static final int MILLION = 1000000;
    
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
	
	public enum GearRatio {
	    R(3.297),
	    N(0.0),
	    ONE(4.696),
	    TWO(3.130),
	    THREE(2.104),
	    FOUR(1.667),
	    FIVE(1.285),
	    SIX(1.000),
	    SEVEN(0.839),
	    EIGHT(0.667),
	    FINAL(2.563);
	    
	    private double value;
	    
	    GearRatio(double value) {
	        this.value = value;
	    }
	    
	    public double getValue() {
	        return value;
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
    	if(rpm > 6200) {
    		Random random = new Random();
            this.rpm = random.nextInt(200) + 6000;
    	}
    	else if(rpm < 0) {
    		this.rpm = 0;
    	}
    	else
    		this.rpm = rpm;
        this.speed = calculateSpeed();
    }
    
    public int getGear() {
        return gear;
    }

    public void setGear(int gear) {
        this.gear = gear;
        //this.speed = calculateSpeed();
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
        	if(this.gear == 0) {
        		setGear(1);
        	}
        	else {
        		setGear(this.gear);
        	}
        	break;
		default:
			break;
        }
    }

    public double getSpeed() {
        return speed;
    }

    private double calculateSpeed() {
        // Speed calculation logic based on RPM and gear
        //double speed = this.rpm * this.gear * 0.01; // TODO: Adjust the calculation formula according to your requirements
    	double gearRatio = GearRatio.values()[this.gear+1].getValue();
    	double finalDrive = GearRatio.FINAL.getValue();
    	//Log.d("GearRatio", ""+GearRatio.values()[this.gear+1].getValue());
    	//Log.d("FinalDrive", ""+finalDrive);
    	this.speed = ((3.78 * 225 * 55) + (4800 * 16)) * this.rpm / (gearRatio * finalDrive * MILLION);
    	//Log.d("speed", ""+this.speed);
        return this.speed;
    }
    
    private int calculateRpm(double speed) {
    	double newRpm = 0;
    	double gearRatio = GearRatio.values()[this.gear+1].getValue();
    	double finalDrive = GearRatio.FINAL.getValue();
    	
    	newRpm = speed / (3.78 * 225 * 55 + 4800 * 16) * (gearRatio * finalDrive * MILLION);
    	Log.d("newRPM : Speed", ""+newRpm+"__"+this.rpm+"__"+this.speed);
    	return (int)newRpm;
    }
	
    public void updateRpm(int rpm) {
        Random random = new Random();
        this.rpm = random.nextInt(21) + 770 + rpm; // Generate a random number between 970 and 990
        //Log.d("UmpdateRPM", ""+rpm);
    }
    
    public void updateRpm(double speed) {
    	this.rpm = calculateRpm(speed);
    }

}
