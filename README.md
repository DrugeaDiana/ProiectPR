# ProiectPR - Drugea Diana Ioana 345C1

## Fisiere proiect:

```ProiectPR.docx``` -> documentul Word cu tot ce tine despre proiect

```/src/ifttt_base_idea.ino```: codul original scris in Arduino ce urmeaza sa fie convertit in micropython (unsure)
contine:
- implementarea pentru actionarea benzii de led in cazul detectarii miscarii
- comunicarea cu senzorii PIR
- obtinerea timpului pentru rasarit si apus si configurarea ceasului intern
- comunicarea prin ifttt cu google assistant pentru pornirea remote a sistemului

```/src/Database.py```: database + program de vizualizare a datelor:
- foloseste un dataframe pentru a pastra datele
- la final se salveaza in ```database.csv```

```/src/Config_Program.py```: program prin care se configureaza placuta
- primeste input de la utilizator cu ora pentru configurare

```/src/Cod_placuta```: codul scris in Arduino pentru placuta ESP32
- implementarea pentru actionarea benzii de led in cazul detectarii miscarii
- comunicarea cu senzorii PIR
- obtinerea configurari prin ascultarea la topicul ```config/system```
- trimiterea de date despre activitate la topicul ```info/activity```

```src/ca.crt```: certificatul folosit pentru programele python pentru a se conecta la mosquitto

