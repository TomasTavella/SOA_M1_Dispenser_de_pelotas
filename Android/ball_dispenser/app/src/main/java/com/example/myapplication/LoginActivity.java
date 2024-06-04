package com.example.myapplication;

import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

public class LoginActivity extends AppCompatActivity {
    private EditText editTextPetName;
    private Button buttonStartGame;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_login);

        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });
        editTextPetName = findViewById(R.id.editTextPetName);
        buttonStartGame = findViewById(R.id.buttonStartGame);

        buttonStartGame.setOnClickListener(v -> validateAndStartGame());
    }

    private void validateAndStartGame() {
        String petName = editTextPetName.getText().toString().trim();

        if (petName.isEmpty()) {
            editTextPetName.setError("El nombre del perro no puede estar vacío");
        } else {
            Intent intent = new Intent(LoginActivity.this, DispenserActivity.class);
            intent.putExtra("PET_NAME", petName);
            startActivity(intent);
        }
    }
}