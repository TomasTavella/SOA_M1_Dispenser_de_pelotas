package com.example.myapplication;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.UUID;

public class DispenserActivity extends AppCompatActivity {
    private static final int REQUEST_BLUETOOTH_PERMISSION = 1;
    private Button btnState;
    private Button btnSensor;
    private TextView tvStateLabel;
    private static final String TAG = "DispenserActivity";
    private final BluetoothAdapter bluetoothAdapter_G = BluetoothAdapter.getDefaultAdapter();
    private BluetoothDevice bluetoothDevice_G;
    private BluetoothSocket bluetoothSocket_G;
    private ConnectedThread connectedThread;
    private static final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    private TextView tvBluetoothStatus;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_dispenser);

        initViews();
        btnState.setOnClickListener(v -> dropBall());
        btnSensor.setOnClickListener(v -> goSensor());
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), this::applyWindowInsets);

        checkPermissions();
        btnState.setBackgroundResource(R.drawable.btn_drop_red);
        simulateArduinoMessages();
    }
    private void initViews() {
        tvBluetoothStatus = findViewById(R.id.tvBluetoothStatus);
        String petName = getIntent().getStringExtra("PET_NAME");
        TextView tvPetName = findViewById(R.id.tvPetName);
        btnState = findViewById(R.id.btnState);
        btnSensor = findViewById(R.id.btnSensor);
        tvPetName.setText("Nombre del Perro: " + (petName != null ? petName : "No especificado"));
        tvStateLabel = findViewById(R.id.tvStateLabel);
    }
    private WindowInsetsCompat applyWindowInsets(View v, WindowInsetsCompat insets) {
        Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
        v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
        return insets;
    }
    private void checkPermissions() {
        String[] permissions = {
                Manifest.permission.BLUETOOTH,
                Manifest.permission.BLUETOOTH_ADMIN,
                Manifest.permission.ACCESS_FINE_LOCATION,
                Manifest.permission.BLUETOOTH_CONNECT
        };

        List<String> permissionsNeeded = new ArrayList<>();

        for (String permission : permissions) {
            if (ActivityCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED) {
                permissionsNeeded.add(permission);
            }
        }
        if (!permissionsNeeded.isEmpty()) {
            ActivityCompat.requestPermissions(this, permissionsNeeded.toArray(new String[0]), REQUEST_BLUETOOTH_PERMISSION);
        } else {
            connectToBluetoothDevice();
        }
    }
    private void connectToBluetoothDevice() {
        int btPermission = ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH);
        int btAdminPermission = ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_ADMIN);
        int locationPermission = ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION);

        if (btPermission != PackageManager.PERMISSION_GRANTED ||
                btAdminPermission != PackageManager.PERMISSION_GRANTED ||
                locationPermission != PackageManager.PERMISSION_GRANTED) {
            Toast.makeText(this, "Faltan permisos para Bluetooth", Toast.LENGTH_SHORT).show();
            return;
        }

        Set<BluetoothDevice> pairedDevices = bluetoothAdapter_G.getBondedDevices();

        for (BluetoothDevice device : pairedDevices) {
            if (device.getName().equals("Galaxy A54 5G")) {
                bluetoothDevice_G = device;
                break;
            }
        }

        if (bluetoothDevice_G != null) {
            try {
                bluetoothSocket_G = bluetoothDevice_G.createRfcommSocketToServiceRecord(MY_UUID);
                bluetoothSocket_G.connect();
                connectedThread = new ConnectedThread(bluetoothSocket_G);
                connectedThread.start();
                updateBluetoothStatus(true);
                Toast.makeText(this, "Conectado al dispositivo Dispenser", Toast.LENGTH_SHORT).show();
            } catch (IOException e) {
                e.printStackTrace();
                updateBluetoothStatus(false);
                Toast.makeText(this, "Error al conectar con Dispenser", Toast.LENGTH_SHORT).show();
            }
        } else {
            updateBluetoothStatus(false);
            Toast.makeText(this, "Dispositivo Dispenser no encontrado", Toast.LENGTH_SHORT).show();
        }
    }
    private void updateBluetoothStatus(boolean isConnected) {
        tvBluetoothStatus.setText(isConnected ? "Bluetooth: On" : "Bluetooth: Off");
    }
    private class ConnectedThread extends Thread {
        private final BluetoothSocket mmSocket;
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;

        public ConnectedThread(BluetoothSocket socket) {
            mmSocket = socket;
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            try {
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) {
                e.printStackTrace();
            }

            mmInStream = tmpIn;
            mmOutStream = tmpOut;
        }

        public void run() {
            byte[] buffer = new byte[1024];
            int bytes;

            while (true) {
                try {

                    bytes = mmInStream.read(buffer);
                    String readMessage = new String(buffer, 0, bytes);
                    runOnUiThread(() -> Toast.makeText(DispenserActivity.this, "Mensaje recibido: " + readMessage, Toast.LENGTH_SHORT).show());
                    handleArduinoMessage(readMessage);
                } catch (IOException e) {
                    updateBluetoothStatus(false);
                    break;
                }
            }
        }
        public void write(String message) {
            try {
                mmOutStream.write(message.getBytes());
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        public void cancel() {
            try {
                mmSocket.close();
                updateBluetoothStatus(false);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
    public void sendMessage(String message) {
        if (connectedThread != null) {
            connectedThread.write(message);
        }
    }
    private void handleArduinoMessage(String message) {
        runOnUiThread(() -> {
            DispenserState state;
            try
            {
                state = DispenserState.valueOf(message.replace(" ", "_").toUpperCase());
            }
            catch (IllegalArgumentException e)
            {
                state = null;
            }

            if (state != null)
            {
                tvStateLabel.setText("Estado: " + state.getLabel());
                btnState.setBackgroundResource(state.getBackgroundResource());
            }
            else
            {
                btnState.setBackgroundResource(R.drawable.btn_drop_red);
                tvStateLabel.setText("Estado: Desconocido");
            }
        });
    }
    private void simulateArduinoMessages() {
        new Thread(() -> {
            try {
                while (true) {
                    Log.d(TAG, "simulateArduinoMessages: Enviando mensaje CHECKING");
                    runOnUiThread(() -> handleArduinoMessage("CHECKING"));
                    Thread.sleep(5000);

                    Log.d(TAG, "simulateArduinoMessages: Enviando mensaje READY");
                    runOnUiThread(() -> handleArduinoMessage("READY"));
                    Thread.sleep(5000);

                    Log.d(TAG, "simulateArduinoMessages: Enviando mensaje DOG DETECTED");
                    runOnUiThread(() -> handleArduinoMessage("DOG DETECTED"));
                    Thread.sleep(5000);

                    Log.d(TAG, "simulateArduinoMessages: Enviando mensaje DROP BALL");
                    runOnUiThread(() -> handleArduinoMessage("DROP BALL"));
                    Thread.sleep(5000);

                    Log.d(TAG, "simulateArduinoMessages: Enviando mensaje END OF SERVICE");
                    runOnUiThread(() -> handleArduinoMessage("END OF SERVICE"));
                    Thread.sleep(5000);
                }
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }).start();
    }
    private void dropBall()
    {
        sendMessage("0");
    }
    private void goSensor()
    {
        Intent intent = new Intent(DispenserActivity.this, SensorActivity.class);
        startActivity(intent);
    }
    public enum DispenserState {
        CHECKING(R.drawable.btn_drop_red, "Checking"),
        READY(R.drawable.btn_drop_green, "Ready"),
        DOG_DETECTED(R.drawable.btn_drop_yellow, "Dog Detected"),
        DROP_BALL(R.drawable.btn_drop_yellow, "Drop Ball"),
        END_OF_SERVICE(R.drawable.btn_drop_yellow, "End of Service");

        private final int backgroundResource;
        private final String label;
        DispenserState(int backgroundResource, String label)
        {
            this.backgroundResource = backgroundResource;
            this.label = label;
        }

        public int getBackgroundResource()
        {
            return backgroundResource;
        }
        public String getLabel()
        {
            return label;
        }
    }
}
