package com.cakru.dodge;

import android.app.Activity;
import android.os.Bundle;
import android.graphics.Color;
import android.view.Gravity;
import android.widget.TextView;

public class MainActivity extends Activity {
    // Memuat library libhello.so (hasil kompilasi C + Rust)
    static {
        System.loadLibrary("hello");
    }

    // Mendeklarasikan fungsi yang ada di sisi Native (C/Rust)
    public native String stringFromRust();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        // Membuat TextView secara dinamis
        TextView tv = new TextView(this);
        tv.setTextSize(24);
        tv.setTextColor(Color.parseColor("#4CAF50")); // Warna Hijau Rust-ish
        tv.setGravity(Gravity.CENTER);
        
        // Memanggil fungsi Rust dan menampilkan teksnya!
        tv.setText(stringFromRust());
        
        setContentView(tv);
    }
}
