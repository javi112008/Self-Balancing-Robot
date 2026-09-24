# Wiring and orientation

## Motor driver

| Arduino Uno pin | L298N input |
| --- | --- |
| D2 | IN1 |
| D3 | IN2 |
| D10 (PWM) | ENA |
| D6 | IN3 |
| D7 | IN4 |
| D11 (PWM) | ENB |
| GND | GND |

Connect one motor to OUT1/OUT2 and the other to OUT3/OUT4. Remove ENA/ENB enable jumpers when using Arduino PWM on those inputs. These are separate from any driver power-regulator jumper.

## IMU

| Arduino Uno connection | MPU6050 module connection |
| --- | --- |
| A4 / SDA | SDA |
| A5 / SCL | SCL |
| GND | GND |
| Supply appropriate to the specific breakout | VCC |

The firmware uses the default MPU6050 address, 0x68 (AD0 low), and does not use the INT pin. Check your specific breakout's supply and I2C voltage compatibility; do not assume a bare MPU6050 tolerates the Uno's 5 V logic.

The pitch calculation uses `atan2(-ax, az)` and integrates `gy / 131.0`. Mount the sensor consistently with these axes. Confirm the measured tilt direction before testing motor corrections; motor lead polarity and physical mounting determine actual wheel direction.

## Power

Use a motor supply appropriate to the motors and driver. Join Arduino, IMU, driver logic, and motor-supply grounds. Do not power motors from the Uno's 5 V pin. Check the particular L298N module's regulator and jumper documentation before connecting its 5 V terminal to another supply.

The recovered code applies gyro offsets (-479, 84, 15) and Z-accelerometer offset 1638. They describe the original setup, not a calibration procedure for a replacement sensor.
