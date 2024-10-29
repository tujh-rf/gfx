package org.tutorial.a002_game_activity

import com.google.androidgamesdk.GameActivity

class MainActivity : GameActivity() {
    companion object {
        init {
            System.loadLibrary("a002_game_activity")
        }
    }
}