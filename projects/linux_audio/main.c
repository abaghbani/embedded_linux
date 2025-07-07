#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#define PCM_DEVICE "default"

#include "i2c.h"
#include "tlv320adc3101.h"

int i2s_init() {
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    int rate = 48000;
    int channels = 2;
    snd_pcm_uframes_t frames = 32;
    int rc;
    char *buffer;
    int size;

    // Open PCM device for recording (capture).
    rc = snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
        return 1;
    }

    // Allocate hardware parameters object
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(pcm_handle, params);
    snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(pcm_handle, params, channels);
    snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, 0);
    snd_pcm_hw_params_set_period_size_near(pcm_handle, params, &frames, 0);

    rc = snd_pcm_hw_params(pcm_handle, params);
    if (rc < 0) {
        fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
        return 1;
    }

    snd_pcm_hw_params_get_period_size(params, &frames, 0);
    size = frames * channels * 2; // 2 bytes per sample (S16_LE)
    buffer = (char *) malloc(size);

    printf("Recording I2S audio from TLV320ADC3101...\n");

    for (int i = 0; i < 100; ++i) {
        rc = snd_pcm_readi(pcm_handle, buffer, frames);
        if (rc == -EPIPE) {
            fprintf(stderr, "overrun occurred\n");
            snd_pcm_prepare(pcm_handle);
        } else if (rc < 0) {
            fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
        } else if (rc != (int)frames) {
            fprintf(stderr, "short read, read %d frames\n", rc);
        }

        // You can process buffer here (contains raw PCM samples)
        printf("Captured %d frames\n", rc);
    }

    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
    free(buffer);
    return 0;
}

int main()
{
    struct I2cDevice dev;

	dev.filename = "/dev/i2c-1";
	dev.addr = 0x18; // 7-bit address : 0x18, 8-bit address: 0x30


	if(i2c_start(&dev) < 0)
	{
		printf("failed to start i2c device: %x\r\n", dev.addr);
		return -1;
	}

	// Initialize the TLV320ADC3101 codec
	tlv320adc3101_init(&dev, 48000, 16);
	printf("TLV320ADC3101 initialized with sample rate 48000Hz and 16-bit sample width.\r\n");
	// Set LED states
	tlv320adc3101_set_led(&dev, 1, 0); // Turn on LED1,

	i2s_init(); // Initialize I2S for audio capture

	// Set LED states
	tlv320adc3101_set_led(&dev, 0, 1);
	printf("LED1 turned on, LED2 turned off.\r\n");

	// Stop the I2C device
	i2c_stop(&dev);

    return 0;
}

