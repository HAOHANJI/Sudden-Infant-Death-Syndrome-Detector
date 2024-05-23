#include <mbed.h>

// Group members: Haohan Ji    NetID:hj2449
//                Guankai Sang NetID:gs3916

/*
Abstract

Our embedded systemr design focuses on detecting SIDS using ZD10-100(
Flexible Membrane Pressure Sensors) from LEANSTAR.

This thin-film pressure sensor is a resistive sensor.
The output resistance decreases with the increase of the pressure applied to the sensor surface.
Through the specific pressure-resistance relationship, the pressure can be measured.
ZD10-100 is mainly used for small pressure detection in a certain area.
With a response point as low as below 20g, it can effectively detect pressure triggers
and pressure changes on the surface of the ZD10-100 sensor.

Function realization

To detect one person's breathing, we strap the pressure sensor on the abdomen using a belt.
When one's inhaling, the belly rises, asserting pressure on the sensor. When one's exhaling,
the belly shrinks. During that, the microcontroller keep track of the voltage data from AnalogIn.
Once the person stop breathing, the voltage data will remain on the high level(approximately 90%),
because of lack of abdomen movement.

Alert
The alert we send after 10s of no breathing is a fast blicking from LD3.

*/

#define READY_FLAG 0b1

// keep track of AnalogIn value
volatile float voltage_percent;
// get AnalogIn from PA_6
AnalogIn ain(PA_6);

Ticker tic;
EventFlags flags;
Timer t;

float last_inhaling = 0;

// threshold value to distinguish breathing from non-breathing
float threshold = 0.5;
// indicator function
bool heartbeat_indicator(volatile float value)
{
  // when voltage is below 50%, inhaling happening
  if (value < threshold)
  {
    return true;
  }
  // when voltage is above 50%, exhaling
  return false;
}

void cb()
{
  flags.set(READY_FLAG);
}

int main()
{
  // sampling rate is every 0.5s
  tic.attach(cb, 500ms);

  float no_heartbeat_time = 0;

  // timer
  t.start();

  // while last exhaling is within 10s, breathe normally
  while (no_heartbeat_time < 10000)
  {
    flags.wait_all(READY_FLAG);
    // read voltage percentage
    voltage_percent = ain.read();

    // if inhaling
    if (heartbeat_indicator(voltage_percent) == true)
    {
      last_inhaling = t.read_ms();
      printf("heartbeat detected at %f ms.\n", last_inhaling);
    }
    else
    {
      no_heartbeat_time = t.read_ms() - last_inhaling;
      printf("no heartbeat for %f ms.\n", no_heartbeat_time);
    }
  }

  // 10s no heartbeat
  // Alert

  printf("--------ALERT--------\n");
  DigitalOut led1(LED1);
  while (1)
  {
    led1 = 1;
    thread_sleep_for(100);
    led1 = 0;
    thread_sleep_for(100);
  }
}
