package org.tutorial.a01_initialization

import com.google.androidgamesdk.GameActivity

class MainActivity : GameActivity() {
    companion object {
        init {
            System.loadLibrary("a01_initialization")
        }
    }
}