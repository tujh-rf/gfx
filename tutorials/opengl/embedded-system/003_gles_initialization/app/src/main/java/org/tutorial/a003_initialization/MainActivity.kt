package org.tutorial.a003_initialization

import com.google.androidgamesdk.GameActivity

class MainActivity : GameActivity() {
    companion object {
        init {
            System.loadLibrary("a003_initialization")
        }
    }
}