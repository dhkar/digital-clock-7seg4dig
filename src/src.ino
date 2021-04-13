#define setButton 13
#define modeButton 0
#define btnDebounceThreshold 50

// Inisiasi Tipe Variabel
// Tipe Data Clock, Inisiasi jam mulai pukul 10:10:00
long count = 36600L;

int hrs = 0;
long hrsBuf = 0;
int mnts = 0;
long mntsBuf = 0;
int secs = 0;
long secsBuf = 0;

// Tipe Data Alarm
long int hrsAlarm = 0L, mntsAlarm = 0L, secsAlarm = 0L;
long triggerAlarmVal;

// Tipe Data Stopwatch
long int SWcount;
long int hrsSW = 0L, mntsSW = 0L, secsSW = 0L;
int stateSW = 41;

// Tipe Data Debouncing Button
int buf = 0;

// State Layer Pertama
short int state;

// State Layer kedua
short int state2 = 0;

void setup()
{

  // Set semua Pin D menjadi output
  for (int i = 0; i < 13; i++)
  {
    pinMode(i, OUTPUT);
  }

  // Output untuk indikasi adanya Alarm
  pinMode(A0, OUTPUT);

  // Output untuk indikasi mode/state
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);

  // Pushbutton Setting sebagai Input Digital
  pinMode(modeButton, INPUT);
  pinMode(setButton, INPUT);

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

  // Nyalakan Interrupt
  interrupts();
}
ISR(TIMER1_OVF_vect)
{
  // Timer counter diinsialisasi
  TCNT1 = 49911;

  // Hanya state 42 yang menambah counter stopwatch
  // state 41 adalah reset stopwatch, state 43 stop stopwatch
  switch (stateSW)
  {
  case 41:
    SWcount = 0;
    break;
  case 42:
    SWcount++;
    break;
  case 43:
    SWcount = SWcount;
    break;
  }

  // Setiap detik akan increment count sebanyak 1
  // count merupakan jumlah detik dari clock
  count++;
}
void loop()
{
  // Menjaga batas nilai detik (count) dalam sehari hingga 86399 (23:59:59)
  // Setelah itu, kembali ke 0 atau (00:00:00)
  if (count > 86399)
  {
    count = 0;
  }

  detectAlarm(30);
  // State Dynamizer, dengan adanya trigger dari button
  detectButton();

  // Parsing data detik (count untuk clock, countSW untuk stopwatch)
  // menjadi data jam:menit:detik
  parseHMS(count, SWcount);
}

void detectAlarm(long int alarmTime)
{
  // Triggering apabila waktu sudah berada di rentang alarm
  triggerAlarmVal = (3600 * hrsAlarm + 60 * mntsAlarm + secsAlarm);
  if ((count > triggerAlarmVal - 1) && (count < alarmTime + triggerAlarmVal) && (count % 2 == 1))
  {
    digitalWrite(A0, HIGH);
  }
  else
  {
    digitalWrite(A0, LOW);
  }
}
// Prosedur detectButton() untuk mendinamisasi state
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
        // State Layer 2 Alarm
        if (state > 30 && state < 34)
        {
          state = state + 1;
        }
        if (state > 33 && state < 40)
        {
          state = 3;
          state2 = 0;
        }

        // State Layer 2 MenitDetik
        if (state > 10 && state < 13)
        {
          state = state + 1;
        }
        if (state > 12 && state < 20)
        {
          state = 1;
          state2 = 0;
        }

        // State Layer 2 Jam
        if (state > 20 && state < 22)
        {
          state = state + 1;
        }
        if (state > 21 && state < 30)
        {
          state = 2;
          state2 = 0;
        }

        // Kembali ke state layer 1 ketika stopwatch
        // ditekan tombol mode
        if (state > 40)
        {
          state = 4;
          state2 = 0;
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
      // State Layer 2 untuk setting Alarm
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

      // State Layer 2 untuk setting menitdetik
      if ((state == 1) && (state2 == 0))
      {
        state2 = 1;
        state = 11;
      }
      if (state == 11)
      {
        mntsBuf++;
        if (mntsBuf > 59)
        {
          mntsBuf = 0;
        }
        count = hrsBuf * 3600 + mntsBuf * 60 + secsBuf;
      }
      if (state == 12)
      {
        secsBuf++;
        if (secsBuf > 59)
        {
          secsBuf = 0;
        }
        count = hrsBuf * 3600 + mntsBuf * 60 + secsBuf;
      }

      // State Layer 2 untuk mengubah jam
      if ((state == 2) && (state2 == 0))
      {
        state2 = 1;
        state = 21;
      }
      if (state == 21)
      {
        hrsBuf++;
        if (hrsBuf > 23)
        {
          hrsBuf = 0;
        }
        count = hrsBuf * 3600 + mntsBuf * 60 + secsBuf;
      }

      if ((state == 4) && (state2 == 0))
      {
        state2 = 1;
        state = 41;
        stateSW = state;
      }
      if (state > 40 && state < 44)
      {
        stateSW = state;
        state = state + 1;
      }
      if (state > 43 && state < 50)
      {
        state = 41;
      }

      buf = 0;
    }
  }
}

// Output Logic Dari State tertentu. stateHandler akan memeriksa state
// dan mengeluarkan output display kepada seven segments
void stateHandler()
{
  if (state == 1)
  {
    printToSevSeg(mnts / 10, mnts % 10, secs / 10, secs % 10, true, true, true, true);
    digitalWrite(A1, LOW);
    digitalWrite(A2, LOW);
  }
  if (state == 2)
  {
    printToSevSeg(0, 0, hrs / 10, hrs % 10, false, false, true, true);
    digitalWrite(A1, LOW);
    digitalWrite(A2, HIGH);
  }
  if (state == 3)
  {
    printToSevSeg(mntsAlarm / 10, mntsAlarm % 10, hrsAlarm / 10, hrsAlarm % 10, true, true, true, true);
    digitalWrite(A1, HIGH);
    digitalWrite(A2, LOW);
  }
  if (state == 4)
  {
    printToSevSeg(mntsSW / 10, mntsSW % 10, secsSW / 10, secsSW % 10, true, true, true, true);
    digitalWrite(A1, HIGH);
    digitalWrite(A2, HIGH);
  }

  if (state == 21)
  {
    printToSevSeg(hrsBuf / 10, hrsBuf % 10, 91, 0, true, true, true, false);
    digitalWrite(A1, LOW);
    digitalWrite(A2, HIGH);
  }
  if (state == 11)
  {
    printToSevSeg(mntsBuf / 10, mntsBuf % 10, 92, 92, true, true, true, true);
    digitalWrite(A1, LOW);
    digitalWrite(A2, LOW);
  }
  if (state == 12)
  {
    printToSevSeg(secsBuf / 10, secsBuf % 10, 5, 0, true, true, true, false);
    digitalWrite(A1, LOW);
    digitalWrite(A2, LOW);
  }

  if (state == 31)
  {
    printToSevSeg(hrsAlarm / 10, hrsAlarm % 10, 91, 0, true, true, true, false);
    digitalWrite(A1, HIGH);
    digitalWrite(A2, LOW);
  }
  if (state == 32)
  {
    printToSevSeg(mntsAlarm / 10, mntsAlarm % 10, 92, 92, true, true, true, true);
    digitalWrite(A1, HIGH);
    digitalWrite(A2, LOW);
  }
  if (state == 33)
  {
    printToSevSeg(secsAlarm / 10, secsAlarm % 10, 5, 0, true, true, true, false);
    digitalWrite(A1, HIGH);
    digitalWrite(A2, LOW);
  }
  if (state == 41)
  {
    printToSevSeg(mntsSW / 10, mntsSW % 10, secsSW / 10, secsSW % 10, true, true, true, true);
    digitalWrite(A1, HIGH);
    digitalWrite(A2, HIGH);
  }
  if (state == 42)
  {
    printToSevSeg(mntsSW / 10, mntsSW % 10, secsSW / 10, secsSW % 10, true, true, true, true);
    digitalWrite(A1, HIGH);
    digitalWrite(A2, HIGH);
  }
  if (state == 43)
  {
    printToSevSeg(mntsSW / 10, mntsSW % 10, secsSW / 10, secsSW % 10, true, true, true, true);
    digitalWrite(A1, HIGH);
    digitalWrite(A2, HIGH);
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

// PrintToSevSeg memiliki parameter 4 digit yang akan dicetak (d1-d4)
// dan digit mana saja yg ingin dinyalakan berupa boolean s1-s4
void printToSevSeg(int d1, int d2, int d3, int d4, bool s1, bool s2, bool s3, bool s4)
{

  // Menyalakan digit 2 (Transistor Vbase LOW, karena Common Cathode)
  // Cetak angka d2 dengan printNum() sebagai "Binary Coded Decimal/BCD"
  if (s2)
  {
    digitalWrite(7, LOW);
    printNum(d2);
    delay(2);
    digitalWrite(7, HIGH);
    digitalWrite(10, LOW);
  }

  // Menyalakan digit 1 (Transistor Vbase LOW, karena Common Cathode)
  // Cetak angka d1 dengan printNum() sebagai "Binary Coded Decimal/BCD"
  if (s1)
  {
    digitalWrite(5, LOW);
    printNum(d1);
    delay(2);
    digitalWrite(5, HIGH);
  }
  // Mencetak Colon Untuk Estetika, sesuai dengan state yang ada
  // Colon merupakan titik 2 pada  7 seg yang biasa berkedip saat clock berjalan
  if ((count % 2 == 1) && (state != 3) && (state / 3 != 10))
  {
    digitalWrite(10, LOW);
  }
  if (count % 2 == 0 && (state != 3) && (state / 3 != 10))
  {
    digitalWrite(10, HIGH);
  }
  if (state == 3)
  {
    digitalWrite(10, HIGH);
  }
  if ((SWcount % 2 == 1 && state > 40) || (state == 4))
  {
    digitalWrite(10, LOW);
  }
  if ((SWcount % 2 == 0 && state > 40) || (state == 4))
  {
    digitalWrite(10, HIGH);
  }
  // Menyalakan digit 4 (Transistor Vbase LOW, karena Common Cathode)
  // Cetak angka d4 dengan printNum() sebagai "Binary Coded Decimal/BCD"
  if (s4)
  {
    digitalWrite(4, LOW);
    printNum(d4);
    delay(2);
    digitalWrite(4, HIGH);
    digitalWrite(10, LOW);
  }

  // Menyalakan digit 3 (Transistor Vbase LOW, karena Common Cathode)
  // Cetak angka d3 dengan printNum() sebagai "Binary Coded Decimal/BCD"
  if (s3)
  {
    digitalWrite(1, LOW);
    printNum(d3);
    delay(2);
    digitalWrite(1, HIGH);
    digitalWrite(10, LOW);
  }
}

// Fungsi printNum sebagai BCD. Dibuat sendiri dengan memerika satu persatu
// port segment a-g dan port Vbase transistor untuk menyalakan digit
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
