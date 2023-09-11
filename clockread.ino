#include <DFRobot_DS1307.h>





/* Constructor */

DFRobot_DS1307 DS1307;



void setup()

{

  Serial.begin(115200);



  // Initialize sensor

  while( !(DS1307.begin()) ){

    Serial.println("Communication with device failed, please check connection");

    delay(3000);

  }

  Serial.println("Begin ok!");

  DS1307.start();



  DS1307.setSqwPinMode(DS1307.eSquareWave_1Hz);



}



void loop()

{

  /**

   *  Get the time from rtc module and convert it to uint16_t

   *  getTimeBuff Array for buffering the obtained time, uint16_t *

   *    getTimeBuff[0] for eSEC type, range: 00-59

   *    getTimeBuff[1] for eMIN type, range: 00-59

   *    getTimeBuff[2] for eHR type, range: 00-23

   *    getTimeBuff[3] for eDOW type, range: 01-07

   *    getTimeBuff[4] for eDATE type, range: 01-31

   *    getTimeBuff[5] for eMTH type, range: 01-12

   *    getTimeBuff[6] for eYR type, range: 2000-2099

   */

  uint16_t getTimeBuff[7] = {0};

  DS1307.getTime(getTimeBuff);

  char outputarr[128];

  sprintf(outputarr, "time: %d/%d/%d-%d %d:%d:%d\r\n",

            getTimeBuff[6],

            getTimeBuff[5],

            getTimeBuff[4],

            getTimeBuff[3],

            getTimeBuff[2],

            getTimeBuff[1],

            getTimeBuff[0]

            );

  Serial.print(outputarr);



 delay(1000);

}
