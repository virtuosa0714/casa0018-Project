/* * CASA0018 Project: Voice Controlled RGB LED (V2 Final Version)
 * Model: Custom CNN (Train from scratch)
 * Features: 6 Classes (Red, Green, Blue, Yellow, Noise, Unknown), 0.8 Confidence Threshold
 */

// ⚠️ Ensure the library name below exactly matches the ZIP library you just imported
#include <CASA0018Project_inferencing.h> 
#include <PDM.h>

/** 1. Struct and buffer definitions */
typedef struct {
    int16_t *buffer;
    uint8_t buf_ready;
    uint32_t buf_count;
    uint32_t n_samples;
} inference_t;

static inference_t inference;
static signed short sampleBuffer[2048];
static bool debug_nn = false; 

/** 2. Hardware initialization (Setup) */
void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("V2 Model Initialized...");

    // Initialize onboard RGB LED pins
    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
    pinMode(LEDB, OUTPUT);
    
    // Initial state: All OFF (Due to common anode design, HIGH is OFF, LOW is ON)
    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, HIGH);

    // Start the microphone
    if (microphone_inference_start(EI_CLASSIFIER_RAW_SAMPLE_COUNT) == false) {
        ei_printf("ERR: Failed to setup audio buffer\r\n");
        return;
    }
}

/** 3. Main loop and AI inference logic (Loop) */
void loop() {
    ei_printf("\n--- Ready for Command (0.5s pause) ---\n");
    delay(500); // Pacing control

    // Record audio
    if (!microphone_inference_record()) {
        ei_printf("ERR: Record failed\n");
        return;
    }

    signal_t signal;
    signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
    signal.get_data = &microphone_audio_signal_get_data;
    ei_impulse_result_t result = { 0 };

    // Run model classification
    EI_IMPULSE_ERROR r = run_classifier(&signal, &result, debug_nn);
    if (r != EI_IMPULSE_OK) {
        ei_printf("ERR: Classifier Error (%d)\n", r);
        return;
    }

    // Process results
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        // Print the current classification and its probability to the serial monitor
        ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);

        // If the probability of a class exceeds the 0.8 threshold
        if (result.classification[ix].value > 0.6) {
            String label = result.classification[ix].label;

            // 1. Color command processing
            if (label == "red" || label == "green" || label == "blue" || label == "yellow") {
                
                // Turn off all LEDs before taking action
                digitalWrite(LEDR, HIGH);
                digitalWrite(LEDG, HIGH);
                digitalWrite(LEDB, HIGH);

                // Turn on the corresponding color based on the command
                if (label == "red") {
                    digitalWrite(LEDR, LOW);
                } else if (label == "green") {
                    digitalWrite(LEDG, LOW);
                } else if (label == "blue") {
                    digitalWrite(LEDB, LOW);
                } else if (label == "yellow") {
                    // Yellow is a hardware mixture of red and green
                    digitalWrite(LEDR, LOW);
                    digitalWrite(LEDG, LOW);
                }

                ei_printf(">>> [ACTION] LED Turned: %s\n", label.c_str());
            } 
            // 2. Environmental noise and unknown sound processing
            else if (label == "unknown" || label == "noise") {
                // Keep silent (turn off all LEDs)
                digitalWrite(LEDR, HIGH);
                digitalWrite(LEDG, HIGH);
                digitalWrite(LEDB, HIGH);
                ei_printf(">>> [SILENCE] Ignored: %s\n", label.c_str());
            }
        }
    }
}

/** 4. Low-level microphone driver callback functions */
static void pdm_data_ready_inference_callback(void) {
    int bytesAvailable = PDM.available();
    int bytesRead = PDM.read((char *)&sampleBuffer[0], bytesAvailable);
    if (inference.buf_ready == 0) {
        for(int i = 0; i < bytesRead>>1; i++) {
            inference.buffer[inference.buf_count++] = sampleBuffer[i];
            if(inference.buf_count >= inference.n_samples) {
                inference.buf_count = 0;
                inference.buf_ready = 1;
                break;
            }
        }
    }
}

static bool microphone_inference_start(uint32_t n_samples) {
    inference.buffer = (int16_t *)malloc(n_samples * sizeof(int16_t));
    if(inference.buffer == NULL) return false;
    inference.buf_count = 0;
    inference.n_samples = n_samples;
    inference.buf_ready = 0;
    PDM.onReceive(&pdm_data_ready_inference_callback);
    PDM.setBufferSize(4096);
    if (!PDM.begin(1, EI_CLASSIFIER_FREQUENCY)) return false;
    PDM.setGain(127);
    return true;
}

static bool microphone_inference_record(void) {
    inference.buf_ready = 0;
    inference.buf_count = 0;
    while(inference.buf_ready == 0) delay(10);
    return true;
}

static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr) {
    numpy::int16_to_float(&inference.buffer[offset], out_ptr, length);
    return 0;
}