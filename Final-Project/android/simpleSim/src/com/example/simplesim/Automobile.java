package com.example.simplesim;

import java.util.Random;

public class Automobile {
	private boolean engineStat;
    private int rpm;
    private Gear gear;
    private double speed;
    
	public enum Gear {
		P("Parking"),
		R("Reverse"),
		N("Neutral"),
		D("Drive"),
		M("Manual")
		;
		private final String label;
		
		Gear(String label){
			this.label = label;
		}
		
		public String label() {
			return label;
		}
	}

    public Automobile() {
    	this.engineStat = false;
        this.rpm = 0;
        this.gear = Gear.P;
        this.speed = calculateSpeed();
    }
    
    public boolean getEngineStat(){
    	return engineStat;
    }
    
    public void setEngineStat(boolean engineStat){
    	this.engineStat = true;
    }

    public int getRpm() {
        return rpm;
    }

    public void setRpm(int rpm) {
        this.rpm = rpm;
        this.speed = calculateSpeed();
    }

    public Gear getGear() {
        return gear;
    }

    public void setGear(Gear gear) {
        this.gear = gear;
        this.speed = calculateSpeed();
    }

    public double getSpeed() {
        return speed;
    }

    private double calculateSpeed() {
        // Speed calculation logic based on RPM and gear
        double speed = rpm * gear.ordinal() * 0.01; // TODO: Adjust the calculation formula according to your requirements
        return speed;
    }
	
    public void updateRpm(int rpm) {
        Random random = new Random();
        rpm = random.nextInt(21) + 970 + rpm; // Generate a random number between 970 and 990
        
    }
}
