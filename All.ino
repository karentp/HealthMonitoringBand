#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino_LSM9DS1.h>
#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/version.h>
#include "model.h"

LiquidCrystal_I2C lcd(0x27, 2,1,0,4,5,6,7,3, POSITIVE);
const float accelerationThreshold = 2.5; // threshold of significant in G's
const int numSamples = 119;

int samplesRead = numSamples;

// global variables used for TensorFlow Lite (Micro)
tflite::MicroErrorReporter tflErrorReporter;

// pull in all the TFLM ops, you can remove this line and
// only pull in the TFLM ops you need, if would like to reduce
// the compiled size of the sketch.
tflite::AllOpsResolver tflOpsResolver;

const tflite::Model *tflModel = nullptr;
tflite::MicroInterpreter *tflInterpreter = nullptr;
TfLiteTensor *tflInputTensor = nullptr;
TfLiteTensor *tflOutputTensor = nullptr;

// Create a static memory buffer for TFLM, the size may need to
// be adjusted based on the model you are using
constexpr int tensorArenaSize = 8 * 1024;
byte tensorArena[tensorArenaSize] __attribute__((aligned(16)));
const int pinBuzzer = 10;
// array to map gesture index to a name
const char *GESTURES[] = {
    "caida",
    "caminando"};

#define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0]))

const int PulseWire = 6;
const int LED13 = 13;
int Threshold = 550;

PulseSensorPlayground pulseSensor;
void setup()
{
    Serial.begin(9600);
    while (!Serial)
        ;
    lcd.begin(16, 2);

    pulseSensor.analogInput(PulseWire);
    pulseSensor.blinkOnPulse(LED13);
    pulseSensor.setThreshold(Threshold);

    if (pulseSensor.begin())
    {
        lcd.print(" Monitoreo de Ritmo Cardiaco");
    }

    // initialize the IMU
    if (!IMU.begin())
    {
        Serial.println("Failed to initialize IMU!");
        while (1)
            ;
    }

    // print out the samples rates of the IMUs
    Serial.print("Accelerometer sample rate = ");
    Serial.print(IMU.accelerationSampleRate());
    Serial.println(" Hz");
    Serial.print("Gyroscope sample rate = ");
    Serial.print(IMU.gyroscopeSampleRate());
    Serial.println(" Hz");

    Serial.println();

    // get the TFL representation of the model byte array
    tflModel = tflite::GetModel(model);
    if (tflModel->version() != TFLITE_SCHEMA_VERSION)
    {
        Serial.println("Model schema mismatch!");
        while (1)
            ;
    }

    // Create an interpreter to run the model
    tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize, &tflErrorReporter);

    // Allocate memory for the model's input and output tensors
    tflInterpreter->AllocateTensors();

    // Get pointers for the model's input and output tensors
    tflInputTensor = tflInterpreter->input(0);
    tflOutputTensor = tflInterpreter->output(0);
}

void loop()
{
    int myBPM = pulseSensor.getBeatsPerMinute(); 

    if (pulseSensor.sawStartOfBeat()) { 
    Serial.println("♥ Pulso Cardiaco detectado ! "); 
    Serial.print("BPM: "); 
    Serial.println(myBPM); 

    lcd.print("Pulso Cardiaco detectado !"); 
    lcd.print("BPM: "); 
    lcd.print(myBPM);
    }
    delay(20); 
    float aX, aY, aZ, gX, gY, gZ;

    // wait for significant motion
    while (samplesRead == numSamples)
    {
        if (IMU.accelerationAvailable())
        {
            // read the acceleration data
            IMU.readAcceleration(aX, aY, aZ);

            // sum up the absolutes
            float aSum = fabs(aX) + fabs(aY) + fabs(aZ);

            // check if it's above the threshold
            if (aSum >= accelerationThreshold)
            {
                // reset the sample read count
                samplesRead = 0;
                break;
            }
        }
    }

    // check if the all the required samples have been read since
    // the last time the significant motion was detected
    while (samplesRead < numSamples)
    {
        // check if new acceleration AND gyroscope data is available
        if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable())
        {
            // read the acceleration and gyroscope data
            IMU.readAcceleration(aX, aY, aZ);
            IMU.readGyroscope(gX, gY, gZ);

            // normalize the IMU data between 0 to 1 and store in the model's
            // input tensor
            tflInputTensor->data.f[samplesRead * 6 + 0] = (aX + 4.0) / 8.0;
            tflInputTensor->data.f[samplesRead * 6 + 1] = (aY + 4.0) / 8.0;
            tflInputTensor->data.f[samplesRead * 6 + 2] = (aZ + 4.0) / 8.0;
            tflInputTensor->data.f[samplesRead * 6 + 3] = (gX + 2000.0) / 4000.0;
            tflInputTensor->data.f[samplesRead * 6 + 4] = (gY + 2000.0) / 4000.0;
            tflInputTensor->data.f[samplesRead * 6 + 5] = (gZ + 2000.0) / 4000.0;

            samplesRead++;
            digitalWrite(10, LOW);
            digitalWrite(8, LOW);
            if (samplesRead == numSamples)
            {
                // Run inferencing
                TfLiteStatus invokeStatus = tflInterpreter->Invoke();
                if (invokeStatus != kTfLiteOk)
                {
                    Serial.println("Invoke failed!");
                    while (1)
                        ;
                    return;
                }

                // Loop through the output tensor values from the model
                for (int i = 0; i < NUM_GESTURES; i++)
                {
                    Serial.print(GESTURES[i]);
                    Serial.print(": ");
                    Serial.println(tflOutputTensor->data.f[i], 6);
                    if (tflOutputTensor->data.f[i] >= 0.4 && GESTURES[i] == "caida")
                    {
                        Serial.println("CAIDAAA");
                        tone(pinBuzzer, 440);
                        delay(1000);

                        // detener tono durante 500ms
                        noTone(pinBuzzer);
                        delay(500);

                        // generar tono de 523Hz durante 500ms, y detenerlo durante 500ms.
                        tone(pinBuzzer, 523, 300);
                        delay(500);
                        tone(pinBuzzer, 440);
                        delay(1000);

                        // detener tono durante 500ms
                        noTone(pinBuzzer);
                        delay(500);

                        // generar tono de 523Hz durante 500ms, y detenerlo durante 500ms.
                        tone(pinBuzzer, 523, 300);
                        delay(500);
                    }
                }
                Serial.println();
            }
        }
    }
}
