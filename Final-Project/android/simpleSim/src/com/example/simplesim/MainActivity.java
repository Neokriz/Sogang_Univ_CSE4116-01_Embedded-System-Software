package com.example.simplesim;

import com.example.simplesim.Controller;
import com.example.simplesim.Automobile;
import com.example.simplesim.VerticalSeekBar;
//import android.R;
import com.example.simplesim.R;

import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.app.Fragment; 
//import android.support.v4.content.ContextCompat;
//import android.annotation.SuppressLint;
//import android.content.Context;
//import android.content.res.Resources;
import android.support.v7.app.ActionBarActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.MotionEvent;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.ToggleButton;

public class MainActivity extends ActionBarActivity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		if (savedInstanceState == null) {
			getSupportFragmentManager().beginTransaction()
					.add(R.id.container, new PlaceholderFragment()).commit();
		}
		
		System.loadLibrary("simple-sim");

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
		TextView speedInfo;
		TextView debug;	TextView debug2;
		
		Handler rpmHandler;
		Runnable rpmRunnable;
		
		Automobile myCar = new Automobile();
		Controller myController = new Controller();
		int gearPos_idx = myCar.getPos().ordinal();
		int guage = 0;
		
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
			speedInfo = (TextView) rootView.findViewById(R.id.speedDpTextView);
			debug = (TextView) rootView.findViewById(R.id.debugTextView1);debug2 = (TextView) rootView.findViewById(R.id.debugTextView2);

			
			Button gearUpBtn = (Button) rootView.findViewById(R.id.gearUpBtn);
			Button gearDownBtn = (Button) rootView.findViewById(R.id.gearDownBtn);
			Button gearMSBtn = (Button) rootView.findViewById(R.id.gearMSBtn);
			ToggleButton ignBtn = (ToggleButton) rootView.findViewById(R.id.ignitionBtn);
			Button throttleBtn = (Button) rootView.findViewById(R.id.throttleBtn);
			Button brakeBtn = (Button) rootView.findViewById(R.id.brakeBtn);
			VerticalSeekBar throttleBar = (VerticalSeekBar) rootView.findViewById(R.id.throttleBar);;


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
			// Ignition button 
			ignBtn.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
				@Override
				public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
					if(isChecked) {
						myController.setIgnitionprocess(1);
						Controller.ignite(myCar, true);
						Log.d("Gear positon:", ""+myCar.getPos());
						gearPositon[myCar.getPos().ordinal()].setTextColor(textColor);
					}
					else {
						myController.setIgnitionprocess(-1);
						Controller.ignite(myCar, false);			
						for(int i=0; i<5; ++i){
							gearPositon[i].setTextColor(Color.BLACK);
						}
					}
					
				}
			});
			// Throttle button 
			throttleBtn.setOnTouchListener(new View.OnTouchListener() {
				@Override
				public boolean onTouch(View v, MotionEvent event) {
			        if (event.getAction() == MotionEvent.ACTION_DOWN) {
						if(myCar.getEngineStat()){
							myController.setAcceleratation(1);
							guage = 100;
						}
			            return true;
			        } else if (event.getAction() == MotionEvent.ACTION_UP) {
						if(myCar.getEngineStat()){
							myController.setAcceleratation(-1);
							guage = 0;
						}
			            return true;
			        }
			        return false;
				}
			});
			// Brake button 
			brakeBtn.setOnTouchListener(new View.OnTouchListener() {
				@Override
				public boolean onTouch(View v, MotionEvent event) {
					if(myCar.getEngineStat()){
						
						DeviceController devCtrl = new DeviceController();
						int ret;
						int fd = devCtrl.openSim();
					    if (fd == -1) {
					    	System.out.print("file open error");
					    }
					    String testData = String.valueOf(myCar.getRpm());
						ret = devCtrl.writeToDevice(fd, testData);

					}
			        if (event.getAction() == MotionEvent.ACTION_DOWN) {

			            return true;
			        } else if (event.getAction() == MotionEvent.ACTION_UP) {

			            return true;
			        }
			        return false;
				}
			});
			// Throttle bar
			throttleBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
				int P_val = 0;
	            @Override
	            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
	                // Handle seek bar progress change
	            	P_val = progress;
	            	guage = P_val;
	            	if(guage > 5){
	            		myController.setAcceleratation(1);
	            	}
	            	else {
	            		myController.setAcceleratation(-1);
	            	}
//	            	if(progress < 20) {
//	            		myController.setAcceleratation(0);
//	            		//guage = 100 - (P_val) * 5;
//	            		guage = P_val;
//	            	}
//	            	else {
//	            		guage = P_val;
//						if(myCar.getEngineStat()){
//							myController.setAcceleratation(1);
//						}
//	            	}
	            }

	            @Override
	            public void onStartTrackingTouch(SeekBar seekBar) {
	                // Handle seek bar touch start
	            }

	            @Override
	            public void onStopTrackingTouch(SeekBar seekBar) {
	                // Handle seek bar touch end
//	            	if(guage < 20) {
//						myController.setAcceleratation(-1);
//	            		//guage = 100 - (P_val) * 5;
//						guage = P_val;
//	            	}
	            }
	        });
			
			//////////////////////////////////////////////////////////////////////////////////////
			
			rpmHandler = new Handler();
			rpmRunnable = new Runnable() {
				@Override
				public void run(){
					if(myController.getIgnitionprocess() == 1) {
						Controller.engineStart(myCar);
						rpmHandler.postDelayed(this, 1);
					}
					else if(myController.getIgnitionprocess() == -1) {
						Controller.engineShutdown(myCar);
						rpmHandler.postDelayed(this, 1);
					}
					else if(myController.getAcceleratation() == 1) {
						Controller.accelerate(myCar, 1, guage);
						rpmHandler.postDelayed(this, 1);
					}
					else if(myController.getAcceleratation() == -1) {
						Controller.accelerate(myCar, -1, guage);
						rpmHandler.postDelayed(this, 1);
					}
					else {
						Controller.idle(myCar);
						rpmHandler.postDelayed(this, 100);
					}
					rpmInfo.setText(String.valueOf(myCar.getRpm()));
					gearInfo.setText(String.valueOf(myCar.getGear()));
					if(myCar.getPos() != Automobile.GearPos.P && myCar.getPos() != Automobile.GearPos.N) {
						speedInfo.setText(String.valueOf((int)Math.round(myCar.getSpeed())));					
					}
					debug.setText(String.valueOf(myController.getAcceleratation()));
					debug2.setText(String.valueOf(guage));
	
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
