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

		int idx = 0;
		
		public PlaceholderFragment() {
		}
		
		TextView rpmInfo;
		TextView[] gearInfo = new TextView[5];
		
		Handler rpmHandler;
		Runnable rpmRunnable;
		
		Automobile myCar = new Automobile();
	
		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
			
			final int textColor = getResources().getColor(R.color.BRIGHT_ORANGE);
			View rootView = inflater.inflate(R.layout.fragment_main, container,
					false);
			
			//final TextView gearInfo = (TextView) rootView.findViewById(R.id.gearPTextView);
			gearInfo[0] = (TextView) rootView.findViewById(R.id.gearPTextView);
			gearInfo[1] = (TextView) rootView.findViewById(R.id.gearRTextView);
			gearInfo[2] = (TextView) rootView.findViewById(R.id.gearNTextView);
			gearInfo[3] = (TextView) rootView.findViewById(R.id.gearDTextView);
			gearInfo[4] = (TextView) rootView.findViewById(R.id.gearMSTextView);
			
			rpmInfo = (TextView) rootView.findViewById(R.id.rpmDptextView);
			
			Button gearUpBtn = (Button) rootView.findViewById(R.id.gearUpBtn);
			Button gearDownBtn = (Button) rootView.findViewById(R.id.gearDownBtn);
			Button gearMSBtn = (Button) rootView.findViewById(R.id.gearMSBtn);
			Button ignBtn = (Button) rootView.findViewById(R.id.ignitionBtn);

			
			//gearInfo.setText(Gear.values()[idx].name());
			gearInfo[0].setTextColor(textColor);
			
			gearUpBtn.setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					
					
					if(idx < 3) {
						idx = gearChange(v, idx, 1);
					}
					Log.d("gearUP", ""+idx);
					//printGear(v, gearInfo, idx);
					for(int i=0; i<4; ++i){
						gearInfo[i].setTextColor(Color.BLACK);
					}
					gearInfo[idx].setTextColor(textColor);
				}
			});
			
			gearDownBtn.setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					if(idx > 0 && idx < 4) {
						idx = gearChange(v, idx, -1);
					}
					Log.d("gearDown", ""+idx);
					//printGear(v, gearInfo, idx);
					for(int i=0; i<5; ++i){
						gearInfo[i].setTextColor(Color.BLACK);
					}
					gearInfo[idx].setTextColor(textColor);
				}
			});

			gearMSBtn.setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					if(idx == 3) {
						idx = gearChange(v, idx, +1);
					}
					else if(idx == 4){
						idx = gearChange(v, idx, -1);
					}
					Log.d("gearManual", ""+idx);
					//printGear(v, gearInfo, idx);
					for(int i=0; i<5; ++i){
						gearInfo[i].setTextColor(Color.BLACK);
					}
					gearInfo[idx].setTextColor(textColor);
				}
			});
			
			ignBtn.setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					myCar.setEngineStat(true);
				}
			});
			
			rpmHandler = new Handler();
			rpmRunnable = new Runnable() {
				@Override
				public void run(){
					rpmInfo.setText(String.valueOf(myCar.getRpm()));
					if(myCar.getEngineStat()) {
						myCar.updateRpm(0);
					}
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
	
	public static int gearChange(View v, int value, int dir){
		return value+dir;
	}
	
	public static void printGear(View v, TextView tview, int value){
		tview.setText(Automobile.Gear.values()[value].name());
	}
}
