#include
const int MPU_addr = 0x68; // I2C direccion del  the MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
float acelerometro_eje_x = 0, acelerometro_eje_y = 0, acelerometro_eje_z = 0, giroscopio_eje_x = 0, giroscopio_eje_y = 0, giroscopio_eje_z = 0;



byte limite_bajo_contador = 0; 
byte limite_alto_contador = 0; 
byte cambio_orientancion_contador = 0; 
int cambio_angulo = 0;
boolean caida = false;     // true si una caida ocurre
boolean limite_bajo = false; // true si se supera el limite bajo
boolean limite_alto = false; // true si se supera el limite alto
boolean cambio_orientancion = false; // true si la orientacion ha cambiado

void setup()
{
    Wire.begin();
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B); // registro PWR_MGMT_1 
    Wire.write(0);    // colocamos el registro en 0 para levantar el MPU-6050
    Wire.endTransmission(true);
    Serial.begin(9600);

    pinMode(11, OUTPUT);
    digitalWrite(11, HIGH);
}


void lectura_mpu()
{
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B); // registro ACCEL_XOUT_H
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr, 14, true); // se solicitan 14 registros
    // Registros
    AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
    GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}

void loop()
{

    lectura_mpu();
    // Se calibra el giroscopio
    giroscopio_eje_x = (GyX + 270) / 131.07;
    giroscopio_eje_y = (GyY - 351) / 131.07;
    giroscopio_eje_z = (GyZ + 136) / 131.07;

    // Se calibra el acelerometro
    acelerometro_eje_x = (AcX - 2050) / 16384.00;
    acelerometro_eje_y = (AcY - 77) / 16384.00;
    acelerometro_eje_z = (AcZ - 1947) / 16384.00;

    // Calcular la amplitud en los tres ejes
    float Raw_amplitud = pow(pow(acelerometro_eje_x, 2) + pow(acelerometro_eje_y, 2) + pow(acelerometro_eje_z, 2), 0.5);
    int amplitud = Raw_amplitud * 10; 
    Serial.println(amplitud);


    if (cambio_orientancion == true)
    {
        cambio_orientancion_contador++;
        Serial.println(cambio_orientancion_contador);
        if (cambio_orientancion_contador >= 10)
        {
            cambio_angulo = pow(pow(giroscopio_eje_x, 2) + pow(giroscopio_eje_y, 2) + pow(giroscopio_eje_z, 2), 0.5);
            delay(10);
            Serial.println(cambio_angulo);
            if ((cambio_angulo >= 0) && (cambio_angulo <= 10)) //Si la orientacion cambia entre 0 y 10 grados
            { 
                caida = true;
                cambio_orientancion = false;
                cambio_orientancion_contador = 0;
                Serial.println(cambio_angulo);
            }
            else // La orientacion vuelve a ser normal
            { 
                cambio_orientancion = false;
                cambio_orientancion_contador = 0;
                Serial.println("CAMBIO DE ORIENTACION DETECTADOo");
            }
        }
    }
    if (caida == true) // Se detecta una caída
    { 
        Serial.println("CAÍDA DETECTADA");
        digitalWrite(11, LOW);
        delay(20);
        digitalWrite(11, HIGH);
        caida = false;
        
    }
    if (limite_alto_contador >= 6)
    { 
        limite_alto = false;
        limite_alto_contador = 0;
        Serial.println("LIMITE ALTO ACTIVADO");
    }
    if (limite_bajo_contador >= 6)
    { 
        limite_bajo = false;
        limite_bajo_contador = 0;
        Serial.println("LIMITE BAJO DESACTIVADO");
    }
    if (limite_alto == true)
    {
        limite_alto_contador++;
        // cambio_angulo=acos(((double)x*(double)bx+(double)y*(double)by+(double)z*(double)bz)/(double)AM/(double)BM);
        cambio_angulo = pow(pow(giroscopio_eje_x, 2) + pow(giroscopio_eje_y, 2) + pow(giroscopio_eje_z, 2), 0.5);
        Serial.println(cambio_angulo);
        if (cambio_angulo >= 30 && cambio_angulo <= 400) // Si la orientacion cambia de 80-100 grados
        { 
            cambio_orientancion = true;
            limite_alto = false;
            limite_alto_contador = 0;
            Serial.println(cambio_angulo);
            Serial.println("CAMBIO DE ORIENTACION DETECTADO");
        }
    }
    if (limite_bajo == true)
    {
        limite_bajo_contador++;
        if (amplitud >= 12)
        { 
            limite_alto = true;
            Serial.println("LIMITE ALTO ACTIVADO");
            limite_bajo = false;
            limite_bajo_contador = 0;
        }
    }
    if (amplitud <= 2 && limite_alto == false)
    { 
        limite_bajo = true;
        Serial.println("LIMITE BAJO ACTIVADO");
    }
    // Delay para no sobrecargar el puerto
    delay(100);
}

