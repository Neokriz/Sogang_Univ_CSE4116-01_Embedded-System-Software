package com.example.simplesim;

import com.example.simplesim.Controller;
import com.example.simplesim.Automobile;
import com.example.simplesim.VerticalSeekBar;
import com.example.simplesim.Needle;
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
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.ToggleButton;

public class MainActivity extends ActionBarActivity {
	public static DeviceController devCtrl = new DeviceController();
	public static int fd;
	public static String[] testData = new String[3];
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		if (savedInstanceState == null) {
			getSupportFragmentManager().beginTransaction()
					.add(R.id.container, new PlaceholderFragment()).commit();
		}
		
		System.loadLibrary("simple-sim");

        // Close the device
		fd = devCtrl.openSim();
	    if (fd == -1) {
	    	System.out.print("file open error");
	    }
		devCtrl.ioctlCmdSim(fd, String.valueOf(0));
		
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
		boolean brakeOn = false;
		public PlaceholderFragment() {
		}

		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
			
			final int textColor = getResources().getColor(R.color.BRIGHT_ORANGE);
			View rootView = inflater.inflate(R.layout.fragment_main, container,	false);
			RelativeLayout parentLayout = (RelativeLayout) rootView.findViewById(R.id.mainFragRelativelayout);

			
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
			final VerticalSeekBar throttleBar = (VerticalSeekBar) rootView.findViewById(R.id.throttleBar);

			final ImageView rpmNeedle = (ImageView) rootView.findViewById(R.id.rpm_needle);
			final ImageView speedNeedle = (ImageView) rootView.findViewById(R.id.speed_needle);
			
			//gearPositon.setText(Gear.values()[gearPos_idx].name());
			
			// Gear Up button 
			gearUpBtn.setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					gearPos_idx = myCar.getPos().ordinal();
					if(myCar.getEngineStat()) {
						if(gearPos_idx <= Automobile.GearPos.N.ordinal()) {
							if(gearPos_idx > Automobile.GearPos.P.ordinal()) {
								gearPos_idx = Controller.gearChange(myCar, gearPos_idx, 1);
								Log.d("gearUP", ""+myCar.getPos()+"(gearPos_idx:"+gearPos_idx+")");
								for(int i=0; i<5; ++i){
									gearPositon[i].setTextColor(Color.BLACK);
								}
								gearPositon[gearPos_idx].setTextColor(textColor);
							}
							else {
								if(brakeOn) {
									gearPos_idx = Controller.gearChange(myCar, gearPos_idx, 1);
									Log.d("gearUP", ""+myCar.getPos()+"(gearPos_idx:"+gearPos_idx+")");
									for(int i=0; i<5; ++i){
										gearPositon[i].setTextColor(Color.BLACK);
									}
									gearPositon[gearPos_idx].setTextColor(textColor);
								}
							}
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
					gearPos_idx = myCar.getPos().ordinal();
					if(myCar.getEngineStat() && gearPos_idx > 0) {
						if(gearPos_idx > Automobile.GearPos.R.ordinal() 
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
						else {
							if(brakeOn) {
								gearPos_idx = Controller.gearChange(myCar, gearPos_idx, -1);
								Log.d("gearDown", ""+myCar.getPos()+"(gearPos_idx:"+gearPos_idx+")");
								for(int i=0; i<5; ++i){
									gearPositon[i].setTextColor(Color.BLACK);
								}
								gearPositon[gearPos_idx].setTextColor(textColor);
							}
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
							myController.setAcceleratation(0);
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
			        if (event.getAction() == MotionEvent.ACTION_DOWN) {
			        	myController.setDeceleration(1);
			        	brakeOn = true;
			            return true;
			        } else if (event.getAction() == MotionEvent.ACTION_UP) {
			        	myController.setDeceleration(0);
			        	brakeOn = false;
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
            	
	            	if(progress < 5) {
	            		myController.setAcceleratation(0);
	            		//guage = 100 - (P_val) * 5;
	            		guage = P_val;
	            	}
	            	else {
	            		guage = P_val;
						if(myCar.getEngineStat()){
							myController.setAcceleratation(1);
						}
	            	}
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
	            	seekBar.setProgress(0);
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
					else if(myController.getAcceleratation() == 0) {
						if(myCar.getRpm() < 770){
							Controller.idle(myCar);
						}
						Controller.accelerate(myCar, -1, guage);
						rpmHandler.postDelayed(this, 1);
					}
					else {
						Controller.idle(myCar);
						rpmHandler.postDelayed(this, 1);
					}
					rpmInfo.setText(String.valueOf(myCar.getRpm()));
					gearInfo.setText(String.valueOf(myCar.getGear()));
					//if(myCar.getPos() != Automobile.GearPos.P && myCar.getPos() != Automobile.GearPos.N) {
						speedInfo.setText(String.valueOf((int)Math.round(myCar.getSpeed())));					
					//}
					debug.setText(String.valueOf(myController.getAcceleratation()));
					debug2.setText(String.valueOf(guage));
					
					float rpmAngle = myCar.getRpm();
					double speedAngle = myCar.getSpeed();
					//TODO rotate needle
					float rotationAngle = ((rpmAngle - 0) / (6200 - 5)) * (270 - 5) + 5;
					rpmNeedle.setRotation(rotationAngle);
					float rotationAngle2 = (((float)speedAngle - 0) / (300 - 5)) * (270 - 5) + 5;
					speedNeedle.setRotation(rotationAngle2);
					//throttleBar.setProgress((int)myCar.getSpeed());
					//Log.d("testData speed", ""+myCar.getSpeed());
				    
				    testData[0] = String.valueOf(myCar.getRpm());
				    testData[1] =  String.valueOf((int)(myCar.getSpeed()));
				    testData[2] = (myCar.getPos() == Automobile.GearPos.M ? 
				    		String.valueOf(myCar.getGear()) : 
				    		String.valueOf(myCar.getPos()));
				    //Log.d("testData", ""+testData[0]+"__"+testData[1]+"__"+testData[2]);
					devCtrl.ioctlSetSim(fd, testData);
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
	    
	    @Override
		public void onDestroy() {
	        super.onDestroy();

	        // Close the device
	        devCtrl.closeSim(fd);
	    }

	}

}
