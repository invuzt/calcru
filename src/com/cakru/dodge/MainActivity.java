package com.cakru.dodge;

import android.app.Activity;
import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class MainActivity extends Activity {
    static {
        System.loadLibrary("hello");
    }

    public native String prosesDiRust(String input);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        EditText input = findViewById(R.id.input_ai);
        Button btn = findViewById(R.id.btn_kirim);
        TextView output = findViewById(R.id.output_ai);

        btn.setOnClickListener(v -> {
            String teks = input.getText().toString();
            String hasil = prosesDiRust(teks);
            output.setText(hasil);
        });
    }
}
