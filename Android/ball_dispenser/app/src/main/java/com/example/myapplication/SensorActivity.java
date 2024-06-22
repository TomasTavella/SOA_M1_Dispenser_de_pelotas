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

public class SensorActivity extends AppCompatActivity implements SensorEventListener {

    // Constants
    private static final float ACCELEROMETRO_VALOR_MAXIMO = 13;
    private static final int POSICION_X = 0;
    private static final int POSICION_Y = 1;
    private static final int POSICION_Z = 2;

    private SensorManager sensorManager_G;
    private Sensor sensor_G;
    private View rootView;
    private boolean isBackgroundWhite = true;  // Track the current background color
    private TextView tvEstadoSensor;
    private int colorIndex = 0;
    private final int[] colores = {Color.RED, Color.GREEN, Color.BLUE, Color.WHITE};
    private final Handler handler = new Handler(Looper.getMainLooper());
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
        tvEstadoSensor = findViewById(R.id.tvSensorStatus);
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
        if (esSacudido(sensorEvent)) {
            cambiarColorDeFondo();
            mostrarEstadoActivado();
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor_G, int i) { }

    private boolean esSacudido(SensorEvent sensorEvent) {
        return (Math.abs(sensorEvent.values[POSICION_X]) > ACCELEROMETRO_VALOR_MAXIMO ||
                Math.abs(sensorEvent.values[POSICION_Y]) > ACCELEROMETRO_VALOR_MAXIMO ||
                Math.abs(sensorEvent.values[POSICION_Z]) > ACCELEROMETRO_VALOR_MAXIMO);
    }

    private void cambiarColorDeFondo() {
        rootView.setBackgroundColor(colores[colorIndex]);
        colorIndex = (colorIndex + 1) % colores.length;
    }
    private void mostrarEstadoActivado() {
        tvEstadoSensor.setText("Estado del sensor: ¡Activado!");
        handler.postDelayed(() -> tvEstadoSensor.setText("Estado del sensor: Esperando..."), 1000);
    }
}