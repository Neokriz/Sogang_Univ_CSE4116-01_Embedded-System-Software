package com.example.simplesim;

public class CarManager {
    private static Automobile myCar;

    public static Automobile getMyCar() {
        if (myCar == null) {
            myCar = new Automobile();
        }
        return myCar;
    }
}