imple-client: simple-client.c
	gcc -Wno-deprecated-declarations -I/usr/local/include/liquid simple-client.c -o simple-client -ljack -lliquid -lm

run:
	LD_LIBRARY_PATH=/usr/local/lib pw-jack ./simple-client

linkports:
	pw-link simple:output_left alsa_output.platform-soc_sound.stereo-fallback:playback_FL
	pw-link simple:output_right alsa_output.platform-soc_sound.stereo-fallback:playback_FR
	pw-link alsa_input.platform-soc_sound.stereo-fallback:capture_FL simple:input_left
	pw-link alsa_input.platform-soc_sound.stereo-fallback:capture_FR simple:input_right

clean:
	rm -f *~ simple-client

include ../scripts/pipewire.mk
