#define TEMP_STEP 1
#define MOIST_STEP 1
#define LIGHT_STEP 1*60*60*1000L  //1 h
#define FERT_FREQ_STEP 1*60*60*1000L  //1 h
#define FERT_TIME_STEP 1

void changeTargetTemp(int schrittfaktor) {
  temperature += TEMP_STEP * schrittfaktor;
  constrain(temperature, MIN_TEMP, MAX_TEMP);
}

void changeMinMoisture(int schrittfaktor, int feldindex = -1) {
  int alt;
  if (feldindex == -1) {
    //Für alle anwenden
    for (int i = 0; i < 4; i++) {
      alt = moistures[i];
      moistures[i] += schrittfaktor * MOIST_STEP;
      if (moistures[i] > ResistanceToMoisture(MAX_MOISTURE) || moistures[i] < ResistanceToMoisture(MIN_MOISTURE)) {
        moistures[i] = alt;
      }
    }
  }
  else {
    alt = moistures[feldindex];
    moistures[feldindex] += schrittfaktor * MOIST_STEP;
    if (moistures[feldindex] > ResistanceToMoisture(MAX_MOISTURE) || moistures[feldindex] < ResistanceToMoisture(MIN_MOISTURE)) {
      moistures[feldindex] = alt;
    }
  }
}

void changeMaxMoisture(int schrittfaktor, int feldindex = -1) {
  //Erst mal nicht implementiert, weil die Pumpen noch nicht unter Kontrolle sind...
}

void changeLightingTime(int schrittfaktor) {
  lichtDauer += LIGHT_STEP * schrittfaktor;
  if (lichtDauer > TagesDauer + 2 * 60 * 60 * 1000L) {
    //negativer Overflow wahrscheinlich
    lichtDauer = TagesDauer;
  }
  if (lichtDauer > TagesDauer) {
    lichtDauer = 0;
  }
}

void changeFertFreq(int schrittfaktor) {
  fertFreq += schrittfaktor * FERT_FREQ_STEP;
  if (fertFreq > 14 * TagesDauer || fertFreq < TagesDauer) {
    fertFreq = 0;
  }
  if (fertFreq > 15 * TagesDauer) {
    //negativer Overflow
    fertFreq = 14 * TagesDauer;
  }
}

void changeFertVol(int schrittfaktor) {
  fertPumpTime += FERT_TIME_STEP * schrittfaktor;
  constrain(fertPumpTime, 0, MAX_FERT_PUMP);
}

