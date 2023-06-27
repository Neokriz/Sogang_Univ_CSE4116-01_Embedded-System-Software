package com.example.simplesim;

import com.example.simplesim.Automobile;
import com.example.simplesim.Automobile.GearRatio;

import android.content.Context;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.text.InputType;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

public class SettingFragment extends Fragment {

    private EditText[] gearEditTexts;
    private Button saveButton;
    private Button resetButton;  
    private Automobile myCar;

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.settings_layout, container, false);

        myCar = CarManager.getMyCar();

        // Initialize EditText fields
        gearEditTexts = new EditText[10];
                
        gearEditTexts[0] = (EditText) rootView.findViewById(R.id.edit_gear_reverse);
        gearEditTexts[1] = (EditText) rootView.findViewById(R.id.edit_gear_1);
        gearEditTexts[2] = (EditText) rootView.findViewById(R.id.edit_gear_2);
        gearEditTexts[3] = (EditText) rootView.findViewById(R.id.edit_gear_3);
        gearEditTexts[4] = (EditText) rootView.findViewById(R.id.edit_gear_4);
        gearEditTexts[5] = (EditText) rootView.findViewById(R.id.edit_gear_5);
        gearEditTexts[6] = (EditText) rootView.findViewById(R.id.edit_gear_6);
        gearEditTexts[7] = (EditText) rootView.findViewById(R.id.edit_gear_7);
        gearEditTexts[8] = (EditText) rootView.findViewById(R.id.edit_gear_8);
        gearEditTexts[9] = (EditText) rootView.findViewById(R.id.edit_final_drive);
        
        for(int i=0; i<10; i++) {
        	gearEditTexts[i].setInputType(InputType.TYPE_CLASS_NUMBER);
        }
        // Initialize save button
        saveButton = (Button) rootView.findViewById(R.id.button_save);
        saveButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
            	saveGearRatios(myCar);
            }
        });
        resetButton = (Button) rootView.findViewById(R.id.button_reset);
        resetButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
            	resetGearRatios(myCar);
            }
        });

        return rootView;
    }

    private void saveGearRatios(Automobile car) {
        double[] gearRatios = new double[10];
        for (int i = 0; i < gearEditTexts.length; i++) {
            String ratioText = gearEditTexts[i].getText().toString().trim();
            if (!ratioText.isEmpty()) {
                double ratio = 0;
                try {
                    ratio = Double.parseDouble(ratioText);
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                    // Handle the case where the input is not a valid number
                }
                if(ratio != 0) {
                	gearRatios[i] = ratio / 1000.0;
                }
            }
        }
        // Set the gear ratios in the GearRatio enum
		if(gearRatios[0] != 0) GearRatio.R.setValue(gearRatios[0]);
		if(gearRatios[1] != 0)GearRatio.ONE.setValue(gearRatios[1]);
		if(gearRatios[2] != 0)GearRatio.TWO.setValue(gearRatios[2]);
		if(gearRatios[3] != 0)GearRatio.THREE.setValue(gearRatios[3]);
		if(gearRatios[4] != 0)GearRatio.FOUR.setValue(gearRatios[4]);
		if(gearRatios[5] != 0)GearRatio.FIVE.setValue(gearRatios[5]);
		if(gearRatios[6] != 0)GearRatio.SIX.setValue(gearRatios[6]);
		if(gearRatios[7] != 0)GearRatio.SEVEN.setValue(gearRatios[7]);
		if(gearRatios[8] != 0)GearRatio.EIGHT.setValue(gearRatios[8]);
		if(gearRatios[9] != 0)GearRatio.FINAL.setValue(gearRatios[9]);

        // Show a toast message or perform any other desired actions to indicate successful saving
        Toast.makeText(getActivity(), "Gear ratios saved successfully", Toast.LENGTH_SHORT).show();
        
        for(int i = 0 ; i<11; i++) {
        	Log.d("NEW gear ratio", ""+Automobile.GearRatio.values()[i].getValue());
        }
    }
    private void resetGearRatios(Automobile car) {

        // Set the default gear ratios.
		GearRatio.R.setValue(3.297);
		GearRatio.ONE.setValue(4.696);
		GearRatio.TWO.setValue(3.130);
		GearRatio.THREE.setValue(2.140);
		GearRatio.FOUR.setValue(1.667);
		GearRatio.FIVE.setValue(1.285);
		GearRatio.SIX.setValue(1.000);
		GearRatio.SEVEN.setValue(0.839);
		GearRatio.EIGHT.setValue(0.667);
		GearRatio.FINAL.setValue(2.563);

        // Show a toast message or perform any other desired actions to indicate successful saving
        Toast.makeText(getActivity(), "Gear ratios reseted successfully", Toast.LENGTH_SHORT).show();

        for(int i = 0 ; i<11; i++) {
        	Log.d("NEW gear ratio", ""+Automobile.GearRatio.values()[i].getValue());
        }
    }

	private Context requireContext() {
		// TODO Auto-generated method stub
		return null;
	}
}