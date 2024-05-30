package net.samples.a01_initialization

import android.view.View
import com.google.androidgamesdk.GameActivity

class MainActivity : GameActivity() {
    companion object {
        init {
            System.loadLibrary("a01_initialization")
        }
    }
}