package com.semicomplete.intellij;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.util.ArrayList;
import java.util.List;

public class Display {
    private static final int LED_COUNT = 33;

    public static class SetLED {
        private final int led;
        private final int red;
        private final int green;
        private final int blue;
        private final int white;

        SetLED(int led, int red, int green, int blue, int white) {
            this.led = led;
            this.red = red;
            this.green = green;
            this.blue = blue;
            this.white = white;
        }

        public String toJSON() {
            return String.format("{ \"led\": %d, \"red\": %d, \"green\": %d, \"blue\": %d, \"white\": %d }", led, red, green, blue, white);
        }
    }

    static void send(String rpcName, String payload) {
        URL url = null;
        try {
            url = new URL("http://192.168.1.161/rpc/" + rpcName);
        } catch (MalformedURLException e) {
            e.printStackTrace();
            return;
        }
        try {
            URLConnection conn = url.openConnection();
            conn.setDoOutput(true);
            System.out.println("Sending request: " + payload);
            conn.getOutputStream().write(payload.getBytes());
            conn.getOutputStream().close();
            System.out.println("  Request sent.");
            while (conn.getInputStream().read() >= 0);
            conn.getInputStream().close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        System.out.println("  Response read");
    }

    static void reportCompilation(int errors, int warnings) {
        List<SetLED> instructions = new ArrayList<>();

        for (int i = 0; i < errors && instructions.size() < LED_COUNT; i++) {
            instructions.add(new SetLED(instructions.size(), 255, 0, 0, 0));
        }

        for (int i = 0; i < warnings && instructions.size() < LED_COUNT; i++) {
            instructions.add(new SetLED(instructions.size(), 128, 128, 0, 0));
        }

        while (instructions.size() < LED_COUNT) {
            instructions.add(new SetLED(instructions.size(), 0,128,0, 0));
        }

        StringBuffer buffer = new StringBuffer();
        buffer.append("{ \"pixels\": [ ");

        int i = 0;
        for (SetLED instruction : instructions) {
            if (i > 0) {
                buffer.append(',');
            }
            i++;

            buffer.append(instruction.toJSON());
        }

        buffer.append("] }");

        send("NeoPixel.SetPixelColorMany", buffer.toString());
    }
}
