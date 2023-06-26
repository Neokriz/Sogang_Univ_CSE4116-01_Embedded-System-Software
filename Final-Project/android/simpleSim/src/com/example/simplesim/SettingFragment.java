package com.example.simplesim;

import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

public class SettingFragment extends Fragment {

    private EditText[] gearEditTexts;
    private Button saveButton;

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.settings_layout, container, false);

        // Initialize EditText fields
        gearEditTexts = new EditText[8];
        gearEditTexts[0] = rootView.findViewById(R.id.edit_gear_1);
        gearEditTexts[1] = rootView.findViewById(R.id.edit_gear_2);
        gearEditTexts[2] = rootView.findViewById(R.id.edit_gear_3);
        // Initialize the remaining gearEditTexts for Gear 4, Gear 5, Gear 6, Gear 7, and Gear 8

        // Initialize save button
        saveButton = rootView.findViewById(R.id.button_save);
        saveButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                saveGearRatios();
            }
        });

        return rootView;
    }

    private void saveGearRatios() {
        double[] gearRatios = new double[8];

        for (int i = 0; i < gearEditTexts.length; i++) {
            String ratioText = gearEditTexts[i].getText().toString();
            double ratio = Double.parseDouble(ratioText);
            gearRatios[i] = ratio;
        }

        // Update gear ratios in the Automobile class
        Automobile automobile = ((MainActivity) requireActivity()).getAutomobile();
        automobile.setGearRatios(gearRatios);

        Toast.makeText(requireContext(), "Gear ratios saved", Toast.LENGTH_SHORT).show();
    }
}