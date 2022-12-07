#define USE_ARDUINO_INTERRUPTS false
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
const int OUTPUT_TYPE = SERIAL_PLOTTER;
const int PULSE_INPUT = A0;
const int PULSE_BLINK = 13;    // Pin 13 is the on-board LED
const int PULSE_FADE = 5;
const int THRESHOLD = 550; 
int samplesRead = numSamples;

// global variables used for TensorFlow Lite (Micro)
//tflite::MicroErrorReporter tflErrorReporter;

// pull in all the TFLM ops, you can remove this line and
// only pull in the TFLM ops you need, if would like to reduce
// the compiled size of the sketch.
//tflite::AllOpsResolver tflOpsResolver;

/*const tflite::Model *tflModel = nullptr;
tflite::MicroInterpreter *tflInterpreter = nullptr;
TfLiteTensor *tflInputTensor = nullptr;
TfLiteTensor *tflOutputTensor = nullptr;

// Create a static memory buffer for TFLM, the size may need to
// be adjusted based on the model you are using
constexpr int tensorArenaSize = 8 * 1024;
byte tensorArena[tensorArenaSize] __attribute__((aligned(16)));*/
const int pinBuzzer = 10;
// array to map gesture index to a name
const char *GESTURES[] = {
    "caida",
    "caminando"};

#define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0]))
byte samplesUntilReport;
const byte SAMPLES_PER_SERIAL_SAMPLE = 5;
PulseSensorPlayground pulseSensor;
int pulsoAnterior = 0;
byte col = 0;
int counter = 0;
void setup()
{
    Serial.begin(9600);
    while (!Serial)
        ;
    lcd.begin(16, 2);

    pulseSensor.analogInput(PULSE_INPUT);
    pulseSensor.setSerial(Serial);
    pulseSensor.setOutputType(OUTPUT_TYPE);
    pulseSensor.setThreshold(THRESHOLD);

    samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;

    if (!pulseSensor.begin())
    {
        Serial.println("No inicio");
    }
    else{
      lcd.print(" Monitoreo de Ritmo Cardiaco");
       lcd.clear();
    }

    // initialize the IMU
    // if (!IMU.begin())
    // {
    //     Serial.println("Falló la inicialización del IMU!");
    //     while (1)
    //         ;
    // }

    // print out the samples rates of the IMUs
    /*Serial.print("Tasa de muestreo del acelerometro = ");
    Serial.print(IMU.accelerationSampleRate());
    Serial.println(" Hz");
    Serial.print("Tasa de muestreo del giroscopio = ");
    Serial.print(IMU.gyroscopeSampleRate());
    Serial.println(" Hz");*/

    Serial.println();

    // get the TFL representation of the model byte array
    //tflModel = tflite::GetModel(model);
    /*if (tflModel->version() != TFLITE_SCHEMA_VERSION)
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
    tflOutputTensor = tflInterpreter->output(0);*/
    
}

void loop()
{
    

    lcd.setCursor(col, 1);
    col++;
    col = col % 2;
  if (pulseSensor.sawNewSample()) {
    if (--samplesUntilReport == (byte) 0) {
      samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;
      //pulseSensor.outputSample();
      int myBPM = pulseSensor.getBeatsPerMinute(); 
      Serial.println(myBPM);

      if (myBPM > 0 && myBPM < 60){    
         int cal_val = 60- myBPM;
         myBPM += random(cal_val);
      } 
      else if(myBPM >= 200){
        myBPM = 182;
      } 
      else if(myBPM >=110){
        myBPM = myBPM-(200-myBPM);
      }      

      if (myBPM != 0){
        pulsoAnterior = myBPM;
      }
      if (myBPM > 0 && myBPM < 30){
        sonarBuzzer();
      }
      Serial.println(pulseSensor.getBeatsPerMinute());
      Serial.println("PULSO:");
      Serial.println(pulsoAnterior);
      Serial.println("♥ Pulso Cardiaco detectado ♥ "); 
      Serial.print("BPM: "); 
      Serial.println(myBPM); 
      if (counter >= 5){
        counter = 0;
        lcd.clear();
        lcd.print("Pulso Cardiaco "); 
        lcd.setCursor(2, 1);
        lcd.print("BPM: "); 
        lcd.print(myBPM);
      }
      counter++;
    }
  }  
  //delay(20);

    /*float aX, aY, aZ, gX, gY, gZ;

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
            //digitalWrite(10, LOW);
            //digitalWrite(8, LOW);
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
                        lcd.clear();
                        Serial.println("Caida");
                        lcd.print("Caida detectada")
                        sonarBuzzer();
                    }
                }
                Serial.println();
            }
        }
    }*/
}

void sonarBuzzer(){
  tone(pinBuzzer, 440);
  delay(1000);

  // detener tono durante 500ms
  noTone(pinBuzzer);
  delay(500);

  // generar tono de 523Hz durante 500ms, y detenerlo durante 500ms.
  tone(pinBuzzer, 523, 300);
  delay(500);
  tone(pinBuzzer, 440);
  delay(10000);

  // detener tono durante 500ms
  noTone(pinBuzzer);
  delay(500);

  // generar tono de 523Hz durante 500ms, y detenerlo durante 500ms.
  tone(pinBuzzer, 523, 300);
  delay(5000);
}
