package io.github.v6cord.vvvvvvce;

import org.libsdl.app.SDLActivity;

public class VCEActivity extends SDLActivity {
    @Override
    protected String[] getLibraries() {
        return new String[] {
                "hidapi",
                "main"
        };
    }
}
