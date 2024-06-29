package com.example.myapplication;

import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;

public class SensorActivity extends AppCompatActivity implements SensorEventListener
{
    private static final float ACCELEROMETER_VALOR_MAXIME = 13;
    private static final int POSITION_X = 0;
    private static final int POSITION_Y = 1;
    private static final int POSITION_Z = 2;
    private static final int DELAY_STATE = 1000;
    private SensorManager sensorManager_G;
    private View rootView;
    private TextView tvStateSensor;
    private int colorIndex = 0;
    private final int[] colours = {Color.RED, Color.GREEN, Color.BLUE, Color.WHITE};
    private final Handler handler = new Handler(Looper.getMainLooper());
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sensor);

        sensorManager_G = (SensorManager) getSystemService(SENSOR_SERVICE);
        Sensor sensor_G = sensorManager_G.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);

        if (sensor_G == null)
        {
            Toast.makeText(this, "No hay acelerómetro en tu dispositivo móvil. :(", Toast.LENGTH_SHORT).show();
            finish();
        } else
        {
            sensorManager_G.registerListener(this, sensor_G, SensorManager.SENSOR_DELAY_NORMAL);
        }
        rootView = findViewById(android.R.id.content);
        tvStateSensor = findViewById(R.id.tvSensorStatus);
    }

    @Override
    protected void onStop()
    {
        super.onStop();
        sensorManager_G.unregisterListener(this);
    }

    @Override
    protected void onResume()
    {
        super.onResume();
        sensorManager_G.registerListener(this, sensorManager_G.getDefaultSensor(Sensor.TYPE_ACCELEROMETER), SensorManager.SENSOR_DELAY_NORMAL);
    }

    @Override
    protected void onPause()
    {
        sensorManager_G.unregisterListener(this);
        super.onPause();
    }

    @Override
    public void onSensorChanged(SensorEvent sensorEvent)
    {
        if (sensorEvent.sensor.getType() != Sensor.TYPE_ACCELEROMETER)
            return;
        if (isShaken(sensorEvent))
        {
            changeBackgroundColor();
            showStatusActivated();
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor_G, int i) { }

    private boolean isShaken(SensorEvent sensorEvent)
    {
        return (Math.abs(sensorEvent.values[POSITION_X]) > ACCELEROMETER_VALOR_MAXIME ||
                Math.abs(sensorEvent.values[POSITION_Y]) > ACCELEROMETER_VALOR_MAXIME ||
                Math.abs(sensorEvent.values[POSITION_Z]) > ACCELEROMETER_VALOR_MAXIME);
    }

    private void changeBackgroundColor()
    {
        rootView.setBackgroundColor(colours[colorIndex]);
        colorIndex = (colorIndex + 1) % colours.length;
    }
    private void showStatusActivated()
    {
        tvStateSensor.setText("State del sensor: ¡Activated!");
        handler.postDelayed(() -> tvStateSensor.setText("Estado del sensor: Esperando..."), DELAY_STATE);
    }
}