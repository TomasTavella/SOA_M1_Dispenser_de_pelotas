package com.example.myapplication;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.pm.PackageManager;
import android.os.Bundle;
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
import java.util.Set;
import java.util.UUID;

public class DispenserActivity extends AppCompatActivity {
    private static final int REQUEST_BLUETOOTH_PERMISSION = 1;
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

        tvBluetoothStatus = findViewById(R.id.tvBluetoothStatus);

        String petName = getIntent().getStringExtra("PET_NAME");
        TextView tvPetName = findViewById(R.id.tvPetName);

        if (petName != null) {
            tvPetName.setText("Nombre del Perro: " + petName);
        } else {
            tvPetName.setText("Nombre del Perro: No especificado");
        }

        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

        // Solicita permisos si no están concedidos
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH) != PackageManager.PERMISSION_GRANTED ||
                ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_ADMIN) != PackageManager.PERMISSION_GRANTED ||
                ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED ||
                ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{
                    Manifest.permission.BLUETOOTH,
                    Manifest.permission.BLUETOOTH_ADMIN,
                    Manifest.permission.ACCESS_FINE_LOCATION,
                    Manifest.permission.BLUETOOTH_CONNECT
            }, 1);
        } else {
            _GetBluetoothDevice();
        }
    }

    private void _GetBluetoothDevice() {
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
            if (device.getName().equals("Dispenser")) {
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
        runOnUiThread(() -> {
            if (isConnected) {
                tvBluetoothStatus.setText("Bluetooth: On");
            } else {
                tvBluetoothStatus.setText("Bluetooth: Off");
            }
        });
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
                    // Aquí puedes manejar los mensajes recibidos
                    runOnUiThread(() -> Toast.makeText(DispenserActivity.this, "Mensaje recibido: " + readMessage, Toast.LENGTH_SHORT).show());
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
}