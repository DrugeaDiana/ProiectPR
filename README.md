# ProiectPR

## Fisiere proiect:

```ProiectPR.docx``` -> documentul Word cu tot ce tine despre proiect, momentan doar "Introducere" + "Arhitectura" 

```/src/ifttt_base_idea.ino```: codul original scris in Arduino ce urmeaza sa fie convertit in micropython (unsure)
contine:
- implementarea pentru actionarea benzii de led in cazul detectarii miscarii
- comunicarea cu senzorii PIR
- obtinerea timpului pentru rasarit si apus si configurarea ceasului intern
- comunicarea prin ifttt cu google assistant pentru pornirea remote a sistemului

```/src/Database.py```: prototip database + program de vizualizare a datelor, idei:
- dtb sa fie un hashtable care ia in functie de timpul in care sunt bagate datele, prob avand o intrare pentru zi si dupa fiecare zi cu un hashtable pt fiecare ora sau direct o lista cu fiecare ora la care a aparut activitate
- while True unde se asteapta input-ul utilizatorul, timp in care programul adauga in dtb orice mesaj ce vine pe topic-ul dat