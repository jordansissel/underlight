package com.semicomplete.intellij;

import org.junit.Test;

import static org.junit.Assert.*;

public class DisplayTest {
    private final int LED_COUNT = 33;

    @Test
    public void displayTest() {
        for (int i = 0; i < LED_COUNT; i++) {
            Display.setLEDColor(i, 255, 0, 0, 0);
        }
    }

    @Test
    public void displayCompilationReport() {
        Display.reportCompilation(5, 10);

        try {
            Thread.currentThread().sleep(500);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        Display.reportCompilation(0, 0);
    }

}