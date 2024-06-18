package com.example.myapplication;

import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;

public class SensorActivity extends AppCompatActivity implements SensorEventListener {

    // Constants
    private static final float ACCELEROMETER_MAX_VALUE = 13;
    private static final int X_POSITION = 0;
    private static final int Y_POSITION = 1;
    private static final int Z_POSITION = 2;

    private SensorManager sensorManager_G;
    private Sensor sensor_G;
    private View rootView;
    private boolean isBackgroundWhite = true;  // Track the current background color

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sensor);

        sensorManager_G = (SensorManager) getSystemService(SENSOR_SERVICE);
        sensor_G = sensorManager_G.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);

        if (sensor_G == null) {
            Toast.makeText(this, "No hay acelerómetro en tu dispositivo móvil. :(", Toast.LENGTH_SHORT).show();
            finish();
        } else {
            sensorManager_G.registerListener(this, sensor_G, SensorManager.SENSOR_DELAY_NORMAL);
        }

        // Get the root view of the activity
        rootView = findViewById(android.R.id.content);
    }

    @Override
    protected void onStop() {
        super.onStop();
        sensorManager_G.unregisterListener(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        // Register listener for the accelerometer
        sensorManager_G.registerListener(this, sensorManager_G.getDefaultSensor(Sensor.TYPE_ACCELEROMETER), SensorManager.SENSOR_DELAY_NORMAL);
    }

    @Override
    protected void onPause() {
        sensorManager_G.unregisterListener(this);
        super.onPause();
    }

    @Override
    public void onSensorChanged(SensorEvent sensorEvent) {
        if (sensorEvent.sensor.getType() != Sensor.TYPE_ACCELEROMETER)
            return;
        if (_IsShaking(sensorEvent)) {
            toggleBackgroundColor();
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor_G, int i) { }

    private boolean _IsShaking(SensorEvent sensorEvent) {
        return (Math.abs(sensorEvent.values[X_POSITION]) > ACCELEROMETER_MAX_VALUE ||
                Math.abs(sensorEvent.values[Y_POSITION]) > ACCELEROMETER_MAX_VALUE ||
                Math.abs(sensorEvent.values[Z_POSITION]) > ACCELEROMETER_MAX_VALUE);
    }

    private void toggleBackgroundColor() {
        if (isBackgroundWhite) {
            rootView.setBackgroundColor(Color.BLACK);
        } else {
            rootView.setBackgroundColor(Color.WHITE);
        }
        isBackgroundWhite = !isBackgroundWhite;
    }
}