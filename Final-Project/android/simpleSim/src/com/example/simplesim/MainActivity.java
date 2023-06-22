package com.example.simplesim;
import java.util.Random;

import com.example.simplesim.Controller;
import com.example.simplesim.Automobile;
//import android.R;
import com.example.simplesim.R;

import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.app.Fragment;
import android.support.v4.content.ContextCompat;
import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.Resources;
import android.support.v7.app.ActionBarActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends ActionBarActivity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		if (savedInstanceState == null) {
			getSupportFragmentManager().beginTransaction()
					.add(R.id.container, new PlaceholderFragment()).commit();
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

	/**
	 * A placeholder fragment containing a simple view.
	 */
	public static class PlaceholderFragment extends Fragment {

		public PlaceholderFragment() {
		}
		
		TextView[] gearPositon = new TextView[5];
		TextView rpmInfo;
		TextView gearInfo;
		
		Handler rpmHandler;
		Runnable rpmRunnable;
		
		Automobile myCar = new Automobile();
		int gearPos_idx = myCar.getPos().ordinal();
		
		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
			
			final int textColor = getResources().getColor(R.color.BRIGHT_ORANGE);
			View rootView = inflater.inflate(R.layout.fragment_main, container,
					false);
			
			//final TextView gearPositon = (TextView) rootView.findViewById(R.id.gearPTextView);
			gearPositon[0] = (TextView) rootView.findViewById(R.id.gearPTextView);
			gearPositon[1] = (TextView) rootView.findViewById(R.id.gearRTextView);
			gearPositon[2] = (TextView) rootView.findViewById(R.id.gearNTextView);
			gearPositon[3] = (TextView) rootView.findViewById(R.id.gearDTextView);
			gearPositon[4] = (TextView) rootView.findViewById(R.id.gearMSTextView);
			
			rpmInfo = (TextView) rootView.findViewById(R.id.rpmDptextView);
			gearInfo = (TextView) rootView.findViewById(R.id.gearDptextView);
			
			
			Button gearUpBtn = (Button) rootView.findViewById(R.id.gearUpBtn);
			Button gearDownBtn = (Button) rootView.findViewById(R.id.gearDownBtn);
			Button gearMSBtn = (Button) rootView.findViewById(R.id.gearMSBtn);
			Button ignBtn = (Button) rootView.findViewById(R.id.ignitionBtn);

			//gearPositon.setText(Gear.values()[gearPos_idx].name());
			
			// Gear Up button 
			gearUpBtn.setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					if(myCar.getEngineStat()) {
						if(gearPos_idx <= Automobile.GearPos.N.ordinal()) {
							gearPos_idx = Controller.gearChange(myCar, gearPos_idx, 1);
							Log.d("gearUP", ""+myCar.getPos()+"(gearPos_idx:"+gearPos_idx+")");
							for(int i=0; i<5; ++i){
								gearPositon[i].setTextColor(Color.BLACK);
							}
							gearPositon[gearPos_idx].setTextColor(textColor);
						}
						else if(gearPos_idx == Automobile.GearPos.M.ordinal()) {
							Controller.shiftUp(myCar);
						}
					}
				}
			});
			// Gear Down button 
			gearDownBtn.setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					if(myCar.getEngineStat()) {
						if(gearPos_idx > Automobile.GearPos.P.ordinal() 
							&& gearPos_idx <= Automobile.GearPos.D.ordinal()) {
							gearPos_idx = Controller.gearChange(myCar, gearPos_idx, -1);
							Log.d("gearDown", ""+myCar.getPos()+"(gearPos_idx:"+gearPos_idx+")");
							for(int i=0; i<5; ++i){
								gearPositon[i].setTextColor(Color.BLACK);
							}
							gearPositon[gearPos_idx].setTextColor(textColor);
						}
						else if(gearPos_idx == Automobile.GearPos.M.ordinal()) {
							Controller.shiftDown(myCar);
						}
					}
				}
			});
			// Gear Manual Shift button 
			gearMSBtn.setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					if(myCar.getEngineStat()){
						if(gearPos_idx == Automobile.GearPos.D.ordinal()) {
							gearPos_idx = Controller.gearChange(myCar, gearPos_idx, +1);
						}
						else if(gearPos_idx == Automobile.GearPos.M.ordinal()){
							gearPos_idx = Controller.gearChange(myCar, gearPos_idx, -1);
						}
						Log.d("gearManual", ""+gearPos_idx);
						//printGear(v, gearPositon, gearPos_idx);
						for(int i=0; i<5; ++i){
							gearPositon[i].setTextColor(Color.BLACK);
						}
						gearPositon[gearPos_idx].setTextColor(textColor);
					}
				}
			});
			
			ignBtn.setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					if(myCar.getEngineStat() == false) {
						Controller.ignite(myCar, true);
						Log.d("Gear positon:", ""+myCar.getPos());
						gearPositon[myCar.getPos().ordinal()].setTextColor(textColor);
					}
					else {
						Controller.ignite(myCar, false);				
						for(int i=0; i<5; ++i){
							gearPositon[i].setTextColor(Color.BLACK);
						}
					}
					
				}
			});
			
			rpmHandler = new Handler();
			rpmRunnable = new Runnable() {
				@Override
				public void run(){
					if(myCar.getEngineStat()) {
						myCar.updateRpm(0);
					}
					else {
						myCar.setRpm(0);
					}
					rpmInfo.setText(String.valueOf(myCar.getRpm()));
					gearInfo.setText(String.valueOf(myCar.getGear()));
					rpmHandler.postDelayed(this, 100);
				}
			};
			

			return rootView;
		}
		
	    @Override
		public void onResume() {
	        super.onResume();
	        rpmHandler.postDelayed(rpmRunnable, 100); // Start the update loop
	    }

	    @Override
		public void onPause() {
	        super.onPause();
	        rpmHandler.removeCallbacks(rpmRunnable); // Stop the update loop
	    }

	}

}
