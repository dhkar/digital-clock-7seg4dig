#define setButton 13
#define modeButton 0
#define btnDebounceThreshold 50
int hrs = 0;
int mnts = 0;
int secs = 0;
long int hrsAlarm = 10L, mntsAlarm = 23L, secsAlarm = 45L;
long int SWcount;
long int hrsSW= 0L, mntsSW = 0L, secsSW = 0L;
bool isStopwatchStops;
long count = 37415L;
short int state;
int buf = 0;
short int state2 = 0;
long triggerAlarmVal;
void setup()
{

  // Set semua Pin D menjadi output
  for (int i = 0; i < 13; i++)
  {
    pinMode(i, OUTPUT);
  }
  pinMode(A0, OUTPUT);
  pinMode(0, INPUT);
  pinMode(13, INPUT);

  // Inisialisasi State
  state = 1;
  // Mematikan interrupt
  noInterrupts();

  // Inisialisasi Timer
  TCCR1A = 0;
  TCCR1B = 0;

  // Set Timer Counter 1 (16 bit) menjadi 49911
  // agar mendapat 1 sekon untuk 16MHz dan prescaler 1024
  TCNT1 = 49911;

  // WGM12 diset menjadi 1 -> falling edge trigger
  // CS12:CS11:CS10 -> 101 -> prescaler 1024
  TCCR1B = 0b00001101;
  // Enable Timer1;
  TIMSK1 = 0b00000001;
  interrupts();
}
ISR(TIMER1_OVF_vect)
{
  // Timer counter diinsialisasi
  TCNT1 = 49911;
  if (state == 41)
  {
    SWcount = 0;
  }
  if (state == 42 || (state != 43) || (state != 41))
  {
    SWcount++;
  }
  if (state == 43)
  {
    SWcount = SWcount;
  }
  // Setiap detik akan menaikkan nilai count untuk ditampilkan SevSeg
  count++;
}
void loop()
{
  // Menjaga batas nilai counter hingga 86399, selebihnya kembali ke 0
  if (count > 86399)
  {
    count = 0;
  }

  // Triggering apabila waktu sudah berada di rentang alarm
  triggerAlarmVal = (3600 * hrsAlarm + 60 * mntsAlarm + secsAlarm);
  if ((count > triggerAlarmVal-1) && (count < 20 + triggerAlarmVal))
  {
    digitalWrite(A0, HIGH);
  }
  else
  {
    digitalWrite(A0, LOW);
  }

  detectButton();
  parseHMS(count,SWcount);
 

}
void detectButton()
{
  if (digitalRead(modeButton) == 0)
  {
    buf = buf + 1;
    if (buf > btnDebounceThreshold)
    {
      if (state2 == 0)
      {
        state = state + 1;
        if (state > 4)
        {
          state = 1;
        }
      }
      if (state2 > 0)
      {
        state2 = 1;
        if (state > 30 && state < 34)
        {
          state = state + 1;
        }
        if (state > 33 && state < 40)
        {
          state = 3;
          state2 = 0;
        }
        if (state > 40)
        {
          state = 4;
        }
      }
      buf = 0;
    }
  }
  if (digitalRead(setButton) == 0)
  {
    buf = buf + 1;
    if (buf > btnDebounceThreshold)
    {
      if ((state == 3) && (state2 == 0))
      {
        state2 = 1;
        state = 31;
      }
      if (state == 31)
      {
        hrsAlarm++;
        if (hrsAlarm > 23)
        {
          hrsAlarm = 0;
        }
      }
      if (state == 32)
      {
        mntsAlarm++;
        if (mntsAlarm > 59)
        {
          mntsAlarm = 0;
        }
      }
      if (state == 33)
      {
        secsAlarm++;
        if (secsAlarm > 59)
        {
          secsAlarm = 0;
        }
      }

      if ((state == 4) && (state2 == 0))
      {
        state2 = 1;
        state = 41;
      }
      if (state == 41)
      {
        state = 42;
      }
      if (state == 42)
      {
        state = 43;
      }
      if (state == 43)
      {
        state = 41;
      }

      buf = 0;
    }
  }
}
void stateHandler()
{
  if (state == 1)
  {
    printToSevSeg(mnts / 10, mnts % 10, secs / 10, secs % 10, true, true, true, true);
  }
  if (state == 2)
  {
    printToSevSeg(0, 0, hrs / 10, hrs % 10, false, false, true, true);
  }
  if (state == 3)
  {
    printToSevSeg(mntsAlarm / 10, mntsAlarm % 10, hrsAlarm / 10, hrsAlarm % 10, true, true, true, true);
    
  }
  if (state == 4)
  {
    printToSevSeg(mntsSW / 10, mntsSW % 10, secsSW / 10, secsSW % 10, true, true, true, true);
  }
  if (state == 31)
  {
    printToSevSeg(hrsAlarm / 10, hrsAlarm % 10, 91, 0, true, true, true, false);
    
  }
  if (state == 32)
  {
    printToSevSeg(mntsAlarm / 10, mntsAlarm % 10, 92, 92, true, true, true, true);
    
  }
  if (state == 33)
  {
    printToSevSeg(secsAlarm / 10, secsAlarm % 10, 5, 0, true, true, true, false);
    
  }
  if (state == 41)
  {
    printToSevSeg(hrsAlarm / 10, hrsAlarm % 10, 91, 0, true, true, true, false);
  }
  if (state == 42)
  {
    printToSevSeg(mntsAlarm / 10, mntsAlarm % 10, 92, 92, true, true, true, true);
  }
  if (state == 43)
  {
    printToSevSeg(secsAlarm / 10, secsAlarm % 10, 5, 0, true, true, true, false);
  }
}

void parseHMS(long clock_rn, long SW_clock)
{

  // Parsing digit count menjadi jam:menit:detik
  hrs = clock_rn / 3600L;
  clock_rn = clock_rn % 3600L;

  mnts = clock_rn % 60L;
  clock_rn = clock_rn / 60L;

  secs = clock_rn % 60L;

// Parsing digit count stopwatch menjadi jam:menit:detik
  hrsSW = SW_clock / 3600L;
  SW_clock = SW_clock % 3600L;

  mntsSW = SW_clock % 60L;
  SW_clock = SW_clock / 60L;

  secsSW = SW_clock % 60L;

    // Mencetak ke Seven Segments
  stateHandler();
}

void printToSevSeg(int d1, int d2, int d3, int d4, bool s1, bool s2, bool s3, bool s4)
{
  // Semua delay 2ms agar terlihat efek simultan pada 7seg
  // Transistor Vbase HIGH agar mematikan semua digit 7seg
  //  digitalWrite(5, s1);
  //  digitalWrite(7, s2);
  //  digitalWrite(1, s3);
  //  digitalWrite(4, s4);

  // Menyalakan (Transistor Vbase LOW) digit 1 , cetak bentuk digit digit1/d1
  // sesuai dengan bentuk datasheet menggunakan printNum() sebagai BCD
  if (s2)
  {
    digitalWrite(7, LOW);
    printNum(d2);
    delay(2);
    digitalWrite(7, HIGH);
    digitalWrite(10, LOW);
  }

  // Menyalakan (Transistor Vbase LOW) digit 1 , cetak bentuk digit digit1/d1
  // sesuai dengan bentuk datasheet menggunakan printNum() sebagai BCD
  if (s1)
  {
    digitalWrite(5, LOW);
    printNum(d1);
    delay(2);
    digitalWrite(5, HIGH);
  }
  if((count%2 == 1) && (state != 3) && (state/3 != 10)){
    digitalWrite(10,LOW);
  }
  if(count%2 == 0 && (state != 3) && (state/3 != 10)){
    digitalWrite(10,HIGH);
  }
  if(state == 3){
    digitalWrite(10,HIGH);
  }

  // Menyalakan (Transistor Vbase LOW) digit 2 , cetak bentuk digit digit2/d2
  // sesuai dengan bentuk datasheet menggunakan printNum sebagai BCD
  if (s4)
  {
    digitalWrite(4, LOW);
    printNum(d4);
    delay(2);
    digitalWrite(4, HIGH);
    digitalWrite(10, LOW);
  }

  // Menyalakan (Transistor Vbase LOW) digit 3 , cetak bentuk digit digit3/d3
  // sesuai dengan bentuk datasheet menggunakan printNum sebagai BCD
  if (s3)
  {
    digitalWrite(1, LOW);
    printNum(d3);
    delay(2);
    digitalWrite(1, HIGH);
    digitalWrite(10, LOW);
  }
}

// Fungsi printNum sebagai BCD
void printNum(int dig)
{
  switch (dig)
  {
  case 0:
    digitalWrite(3, HIGH);
    digitalWrite(2, HIGH);
    digitalWrite(6, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(11, HIGH);
    digitalWrite(12, HIGH);
    digitalWrite(8, LOW);
    break;
  case 1:
    digitalWrite(3, LOW);
    digitalWrite(2, LOW);
    digitalWrite(6, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(11, LOW);
    digitalWrite(12, LOW);
    digitalWrite(8, LOW);
    break;
  case 2:
    digitalWrite(3, LOW);
    digitalWrite(2, HIGH);
    digitalWrite(6, HIGH);
    digitalWrite(9, LOW);
    digitalWrite(11, HIGH);
    digitalWrite(12, HIGH);
    digitalWrite(8, HIGH);
    break;
  case 3:
    digitalWrite(3, LOW);
    digitalWrite(2, HIGH);
    digitalWrite(6, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(11, HIGH);
    digitalWrite(12, LOW);
    digitalWrite(8, HIGH);
    break;

  case 4:
    digitalWrite(3, HIGH);
    digitalWrite(2, LOW);
    digitalWrite(6, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(11, LOW);
    digitalWrite(12, LOW);
    digitalWrite(8, HIGH);
    break;
  case 5:
    digitalWrite(3, HIGH);
    digitalWrite(2, HIGH);
    digitalWrite(6, LOW);
    digitalWrite(9, HIGH);
    digitalWrite(11, HIGH);
    digitalWrite(12, LOW);
    digitalWrite(8, HIGH);
    break;
  case 6:
    digitalWrite(3, HIGH);
    digitalWrite(2, HIGH);
    digitalWrite(6, LOW);
    digitalWrite(9, HIGH);
    digitalWrite(11, HIGH);
    digitalWrite(12, HIGH);
    digitalWrite(8, HIGH);
    break;
  case 7:
    digitalWrite(3, LOW);
    digitalWrite(2, HIGH);
    digitalWrite(6, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(11, LOW);
    digitalWrite(12, LOW);
    digitalWrite(8, LOW);
    break;
  case 8:
    digitalWrite(3, HIGH);
    digitalWrite(2, HIGH);
    digitalWrite(6, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(11, HIGH);
    digitalWrite(12, HIGH);
    digitalWrite(8, HIGH);
    break;
  case 9:
    digitalWrite(3, HIGH);
    digitalWrite(2, HIGH);
    digitalWrite(6, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(11, HIGH);
    digitalWrite(12, LOW);
    digitalWrite(8, HIGH);
    break;
  // HURUF H
  case 91:
    digitalWrite(3, HIGH);
    digitalWrite(2, LOW);
    digitalWrite(6, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(11, LOW);
    digitalWrite(12, HIGH);
    digitalWrite(8, HIGH);
    break;
  // HURUF M
  case 92:
    digitalWrite(3, HIGH);
    digitalWrite(2, HIGH);
    digitalWrite(6, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(11, LOW);
    digitalWrite(12, HIGH);
    digitalWrite(8, LOW);
    break;
  }
}
