package com.example.simplesim;

import com.example.simplesim.Controller;
import com.example.simplesim.Automobile;
import com.example.simplesim.VerticalSeekBar;
//import com.example.simplesim.Needle;
//import com.example.simplesim.InterruptDetector;
//import android.R;
import com.example.simplesim.R;

import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.support.v4.app.Fragment; 
//import android.support.v4.content.ContextCompat;
//import android.annotation.SuppressLint;
//import android.content.Context;
//import android.content.res.Resources;
import android.support.v4.app.FragmentTransaction;
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
	public static int fd, fd2;
	public static String[] fpgaData = new String[3];

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
		fd2 = devCtrl.openSimInt();
	    if (fd == -1) {
	    	System.out.print("file1 open error");
		    Log.d("2 device1 opened error", "");
	    }
	    else if (fd2 == -1) {
	    	System.out.print("file2 open error");
		    Log.d("2 device2 opened  error", "");
	    }
	    Log.d("2 device opened without error", "");
	    
		devCtrl.ioctlCmdSim(fd, String.valueOf(0));
		//devCtrl.readInterrupt(fd2, String.valueOf(0));

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
            // Navigate to the SettingFragment when the "Settings" button is clicked
            Fragment settingFragment = new SettingFragment();
            FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();
            transaction.replace(R.id.container, settingFragment);
            transaction.addToBackStack(null);  // Add to back stack to enable back navigation
            transaction.commit();
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
		Handler intHandler;
		Runnable intRunnable;
		
		Automobile myCar = CarManager.getMyCar();
		Controller myController = new Controller();
		int gearPos_idx = myCar.getPos().ordinal();
		int guage = 0;
		int interruptGearChange = 0;
		boolean simulateRPM = true;
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
			
			final Button gearUpBtn = (Button) rootView.findViewById(R.id.gearUpBtn);
			final Button gearDownBtn = (Button) rootView.findViewById(R.id.gearDownBtn);
			final Button gearMSBtn = (Button) rootView.findViewById(R.id.new_gearMSBtn);
			Button dummy_gearMSBtn = (Button) rootView.findViewById(R.id.gearMSBtn);
			ToggleButton ignBtn = (ToggleButton) rootView.findViewById(R.id.ignitionBtn);
			Button throttleBtn = (Button) rootView.findViewById(R.id.throttleBtn);
			Button brakeBtn = (Button) rootView.findViewById(R.id.brakeBtn);
			final VerticalSeekBar throttleBar = (VerticalSeekBar) rootView.findViewById(R.id.throttleBar);

			final ImageView rpmNeedle = (ImageView) rootView.findViewById(R.id.rpm_needle);
			final ImageView speedNeedle = (ImageView) rootView.findViewById(R.id.speed_needle);
			final ImageView accelPedal = (ImageView) rootView.findViewById(R.id.accel_img);
			final ImageView brakePedal = (ImageView) rootView.findViewById(R.id.brake_img);
			final ImageView engineBtn_blue = (ImageView) rootView.findViewById(R.id.engine_button_blue);
			final ImageView brakeLamp_off = (ImageView) rootView.findViewById(R.id.brake_lamp_off);
			final ImageView rpmMeter = (ImageView) rootView.findViewById(R.id.rpm_background);
			final ImageView speedMeter = (ImageView) rootView.findViewById(R.id.speed_background);
			final ImageView rpmMeterGlow = (ImageView) rootView.findViewById(R.id.rpm_background_glow);
			final ImageView speedMeterGlow = (ImageView) rootView.findViewById(R.id.speed_background_glow);

			engineBtn_blue.setVisibility(View.INVISIBLE);
			dummy_gearMSBtn.setVisibility(View.INVISIBLE);
			//rpmMeterGlow.setVisibility(View.INVISIBLE);
			speedMeterGlow.setVisibility(View.INVISIBLE);

			brakeLamp_off.setColorFilter(Color.argb(10, 0, 0, 0));
			
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
						if(gearPos_idx == Automobile.GearPos.N.ordinal() 
							|| gearPos_idx == Automobile.GearPos.D.ordinal()) {
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
							gearPos_idx = Controller.gearChange(myCar, gearPos_idx, -1);//TODO
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
						engineBtn_blue.setVisibility(View.VISIBLE);
						myController.setIgnitionprocess(1);
						Controller.ignite(myCar, true);
						Log.d("Gear positon:", ""+myCar.getPos());
						gearPositon[myCar.getPos().ordinal()].setTextColor(textColor);
					}
					else {
						engineBtn_blue.setVisibility(View.INVISIBLE);
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
			                accelPedal.setColorFilter(Color.argb(150, 0, 0, 0));
							guage = 100;
						}
			            return true;
			        } else if (event.getAction() == MotionEvent.ACTION_UP) {
						if(myCar.getEngineStat()){
							myController.setAcceleratation(0);
							accelPedal.setColorFilter(0);
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
		                brakePedal.setColorFilter(Color.argb(150, 0, 0, 0));
		                brakeLamp_off.setVisibility(View.INVISIBLE);
			        	brakeOn = true;
			            return true;
			        } else if (event.getAction() == MotionEvent.ACTION_UP) {
			        	myController.setDeceleration(0);
			        	brakePedal.setColorFilter(0);
		                brakeLamp_off.setVisibility(View.VISIBLE);
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
			rpmMeter.setOnClickListener(new View.OnClickListener() {
			    @Override
			    public void onClick(View v) {
			    	rpmMeterGlow.setVisibility(View.VISIBLE);
			    	speedMeterGlow.setVisibility(View.INVISIBLE);
			    	simulateRPM = true;
			    }
			});
			speedMeter.setOnClickListener(new View.OnClickListener() {
			    @Override
			    public void onClick(View v) {
			    	speedMeterGlow.setVisibility(View.VISIBLE);
			    	rpmMeterGlow.setVisibility(View.INVISIBLE);
			    	simulateRPM = false;
			    }
			});
			
			//////////////////////////////////////////////////////////////////////////////////////
			
			rpmHandler = new Handler();
			rpmRunnable = new Runnable() {
				@Override
				public void run(){
					if(myController.getIgnitionprocess() == 1) {
						Controller.engineStart(myCar);
					}
					else if(myController.getIgnitionprocess() == -1) {
						Controller.engineShutdown(myCar);
					}
					else if(myController.getDeceleratation() == 1) {
						Controller.deceleratation(myCar, brakeOn);
					}
					else if(myController.getAcceleratation() == 1) {
						Controller.accelerate(myCar, 1, guage, brakeOn);
					}
					else if(myController.getAcceleratation() == 0) {
						if(myCar.getRpm() < 770){
							Controller.idle(myCar, brakeOn);
						}
						Controller.accelerate(myCar, -1, guage, brakeOn);
					}
					else {
						Controller.idle(myCar, brakeOn);
					}
					rpmHandler.postDelayed(this, 10);
					
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
					float rotationAngle = ((rpmAngle - 0) / (6200 - 5)) * (280 - 5) + 5;
					rpmNeedle.setRotation(rotationAngle);
					float rotationAngle2 = (((float)speedAngle - 0) / (300 - 5)) * (280 - 5) + 5;
					speedNeedle.setRotation(rotationAngle2);
					//throttleBar.setProgress((int)myCar.getSpeed());
					//Log.d("fpgaData speed", ""+myCar.getSpeed());
				    
				    fpgaData[0] = String.valueOf(myCar.getRpm());
				    fpgaData[1] = simulateRPM ? String.valueOf(0) : String.valueOf((int)(myCar.getSpeed()));
				    fpgaData[2] = (myCar.getPos() == Automobile.GearPos.M ? 
				    		String.valueOf(myCar.getGear()) : 
				    		String.valueOf(myCar.getPos()));
				    //Log.d("fpgaData", ""+fpgaData[0]+"__"+fpgaData[1]+"__"+fpgaData[2]);
					devCtrl.ioctlSetSim(fd, fpgaData);
					
					//Gear change by interrupt button
			        if(interruptGearChange == 1) {
			        	gearDownBtn.performClick();
			        	interruptGearChange = 0;
			        }
			        else if(interruptGearChange == 2) {
			        	gearUpBtn.performClick();
			        	interruptGearChange = 0;
			        }
			        else if(interruptGearChange == 3) {
			        	gearMSBtn.performClick();
			        	interruptGearChange = 0;
			        }
			   
				}
			};
			
			HandlerThread intThread = new HandlerThread("IntThread");
			intThread.start();
			intHandler = new Handler(intThread.getLooper());
			intRunnable = new Runnable() {
			    @Override
			    public void run() {
			    	interruptGearChange = devCtrl.readInterrupt(fd2, "0000");
			        //Log.d("retValue from readInterrupt", ""+interruptGearChange);
			        
			        intHandler.postDelayed(this, 100);
			    }
			};
			
			rpmHandler.post(rpmRunnable);
			intHandler.post(intRunnable);
			
			return rootView;
		}
		
	    @Override
		public void onResume() {
	        super.onResume();
	        rpmHandler.postDelayed(rpmRunnable, 10); // Start the update loop
	        intHandler.postDelayed(intRunnable, 100); 	// Stop the update loop
	    }

	    @Override
		public void onPause() {
	        super.onPause();
	        rpmHandler.removeCallbacks(rpmRunnable); // Stop the update loop
	        intHandler.removeCallbacks(intRunnable); // Stop the update loop
	    }
	    
	    @Override
		public void onDestroy() {
	        super.onDestroy();

	        // Close the device
	        devCtrl.closeSim(fd);
	        devCtrl.closeSimInt(fd2);
	    }

	}

}
