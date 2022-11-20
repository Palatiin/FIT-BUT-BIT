Implementační dokumentace k 2. úloze do IPP 2021/2022<br/>
Jméno a příjmení: Matúš Remeň<br/>
Login: xremen01

## Dokumentácia *interpret.py*

### trieda **Stack**
Implementuje dátovú štruktúru zásobník a používa sa
pri vykonávaní inštrukcií `PUSHFRAME`, `POPFRAME`, `PUSHS`, `POPS`,
`CALL` a `RETURN`. V triede **Interpret** sa vyskytujú 3 inštancie
tejto triedy - *frame_stack*, *data_stack* a *call_stack*.<br/>
Metódy:
- *push*
- *pop*
- *top*

### trieda **InputQueue**
Implementácia dátovej štruktúry fronta s účelom spracovávania
užívatelského vstupu pre inštrukciu `READ`, keď dáta vstupujú
do skriptu v súbore cez prepínač *--input*. Pri inicializácii
sa načítajú všetky dáta zo súboru a pri vykonávaní inštrukcie
`READ` sa vždy požiada o ďalší vstup-riadok pomocou metódy `next()`.
Ak je fronta prázdna, metóda `next()` vracia `None`, ktorý sa
následne podla zadania konvertuje na `nil`.<br/>
Metódy:
- *next*

### trieda **Frame**
Spravuje premenné definované v nej.
Trieda obsahuje slovník a implementuje metódy pre prácu s ním.
Tieto metódy sú rozšírením základných built-in metód jazyka
Python o kontroly vyplývajúce zo zadania projektu, napr.
kontrola redefinície premennej - vedie k ukončeniu programu
so zadaným návratovým kódom.<br/>
Metódy:
- *def_var*
- *set_value*
- *get_value*

### trieda **InsArguments**
Spravuje arguementy inštrukcií. Pri inicializácii parsuje
s xml tagu inštrukcie jej argumenty, ukladá ich a kontroluje
duplicitu a poradie argumentov. Umožňuje prístup k hodnotám
cez slovník alebo cez atribúty `arg1`, `arg2` a	`arg3`.<br/>
Metódy:
- *validate*

### trieda **Value**
Zoskupuje dátový typ a hodnotu premmennej. Umožňuje prístup
k týmto dátam cez atribúty `type` a `value`.

### trieda **Instruction**
Ukladá si operačný kód inštrukcie a jej argumenty. Obsahuje
implementácie inštrukcií zo zadania projektu a pomocné
metódy na uľahčenie kontroly a práce s ich argumentami.<br/>
Metódy:
- *execute*
- *check_var*
- *check_symb*
- *check_label*
- *check_arithmetic*
- *check_relational*
- *check_bool*
- *... implementácie inštrukcií*

### trieda **Interpret**
Singleton. Pri inicializácii objektu sa uložia do atribútov
vstupné súbory pre zdrojový kód a užívatelský vstup.
Ak je užívatelský vstup zo súboru iného ako `stdin`, tak sa
inicializuje trieda **InputQueue**.<br/>
Metóda *process* riadi celý interpret. Kroky metódy:
1. *zostavenie XML ElementTree*
2. *parsovanie inštrukcií a ich argumentov z ElementTree*
3. *validácia poradových čísiel inštrukcií*
4. *vyhodnotenie inštrukcií 'LABEL'* - inštrukcie `LABEL` sú po ich vyhodnotení nahradené prázdnou inštrukciou `NOP` (mimo zadania), aby sa zabránilo opätovnému vyhodnocovaniu
5. *vyhodnotenie ostatných inštrukcií*
Metódy:
- *stats*
- *process*
- *_parse_instruction*
- *_check_instructions*

