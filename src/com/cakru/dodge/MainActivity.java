package com.cakru.dodge;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

public class MainActivity extends Activity {
    static {
        System.loadLibrary("hello");
    }

    public native String prosesDiRust(String input);
    public native void setModelPath(String path);

    private TextView outputAi;
    private static final int PICK_FILE_CODE = 101;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        outputAi = findViewById(R.id.output_ai);
        EditText inputAi = findViewById(R.id.input_ai);
        Button btnKirim = findViewById(R.id.btn_kirim);
        ImageButton btnSettings = findViewById(R.id.btn_settings);

        btnSettings.setOnClickListener(v -> {
            Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
            intent.setType("*/*");
            startActivityForResult(intent, PICK_FILE_CODE);
        });

        btnKirim.setOnClickListener(v -> {
            String txt = inputAi.getText().toString();
            if (!txt.isEmpty()) {
                outputAi.setText("AI sedang berpikir...");
                // Note: Di produksi, ini harus dijalankan di Thread berbeda agar tidak ANR
                String respons = prosesDiRust(txt);
                outputAi.setText(respons);
            }
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == PICK_FILE_CODE && resultCode == RESULT_OK && data != null) {
            Uri uri = data.getData();
            try {
                String path = copyToInternal(uri);
                setModelPath(path);
                outputAi.setText("Model Aktif: " + new File(path).getName());
                Toast.makeText(this, "Model berhasil dimuat!", Toast.LENGTH_SHORT).show();
            } catch (Exception e) {
                outputAi.setText("Gagal: " + e.getMessage());
            }
        }
    }

    private String copyToInternal(Uri uri) throws Exception {
        File dest = new File(getFilesDir(), "current_model.gguf");
        InputStream is = getContentResolver().openInputStream(uri);
        FileOutputStream os = new FileOutputStream(dest);
        byte[] buffer = new byte[4096];
        int bytesRead;
        while ((bytesRead = is.read(buffer)) != -1) {
            os.write(buffer, 0, bytesRead);
        }
        is.close();
        os.close();
        return dest.getAbsolutePath();
    }
}
