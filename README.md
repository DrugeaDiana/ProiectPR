# ProiectPR - Drugea Diana Ioana 345C1

## Fisiere proiect:

```ProiectPR.docx``` -> documentul Word cu tot ce tine despre proiect
```ProiectPR.pdf``` -> documentul PDF cu tot ce tine despre proiect

```/src/Database.py```: database + program de vizualizare a datelor:
- foloseste un dataframe pentru a pastra datele
- asculta topicul ```info/activity``` si de fiecare data cand se trimite un mesaj,
il adauga in dataframe.
- la inchiderea programului se salveaza in ```database.csv```

```/src/Config_Program.py```: program prin care se configureaza placuta
- primeste input de la utilizator cu ora pentru configurare, oferind mai multe optiuni
- acesta dupa trimite configurarea pe ```config/system``` in format json

```/src/Cod_placuta```: codul scris in Arduino pentru placuta ESP32
- implementarea pentru actionarea benzii de led in cazul detectarii miscarii
- comunicarea cu senzorii PIR
- obtinerea configurari prin ascultarea la topicul ```config/system```
- trimiterea de date despre activitate la topicul ```info/activity```

```src/ca.crt```: certificatul folosit pentru programele python pentru a se conecta la mosquitto

