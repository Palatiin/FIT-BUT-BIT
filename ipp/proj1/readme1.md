Implementační dokumentace k 1. úloze do IPP 2021/2022<br/>
Jméno a příjmení: Matúš Remeň<br/>
Login: xremen01

## Dokumentácia *parse.php*
Skript obshahuje triedu **Parser**, ktorej metódy majú na starosti
parsovanie a kontrolovanie inštrukcií a ich parametrov, skladanie
a výpis reprezentácie programu v XML.

## trieda **Parser**
### Atribúty:
- *xml* - obsahuje výstupnú reprezentáciu programu v XML
- *order* - počítadlo elementov - inštrukcií pre tvorbu výstupného XML

### Konštruktor
Inicializácia objektu, atribútov a kontrola volitelných argumentov skriptu:
Ako parametre prijíma *argc* a *argv* - vstupné argumenty skriptu.

### Metóda *parse* (public)
Po skonštruovaní objektu sa volá jeho hlavná a jediná verejná metóda *parse()*. Prechádza
zdrojový kód. Najprv skontroluje hlavičku **.IPPcode22**, ktorú musí obsahovať
každý program v tomto jazyku, potom riadok po riadku analyzuje inštrukcie
a ich parametre. V načítanom riadku sa oddelí blok komentáru, ak je prítomný,
a zavolá sa metóda *parse_ins()*, ktorá parsuje inštrukciu. Nakoniec vypíše vyskladané
XML.

### Metóda *parse_ins* (private)
Parametre:
- *splitted_line* - pole, riadok rozdelený podla medzier<br/>

Inštrukcie som rozdelil do skupín podla toho, aké a kolko vstupných argumentov
prijímajú. Vo výstupnom XML sa vytvori nový element inštrukcie, ktorá sa za využitia
regulárných výrazov identifikuje a skontrolujú sa jej argumenty.
Ak sa jedná o neznámu inštrukciu, ukončí skript s návratovým kódom 22.

### Metóda *check_params* (private)
Kontrola parametrov inštrukcie.<br/>
Parametre:
- *splitted_line*
- *param_count* - očakávaný počet parametrov inštrukcie
- *param_type* - pole očakávaných typov parametrov<br/>

Podla dĺžky pola `splitted_line - 1` sa zistí kolko má daná inštrukcia
zadaných parametrov a porovná sa s očakávaným počtom parametrov, ak
sa nezhodujú, skript končí s návratovým kódom 23.<br/>

Nasleduje parsovanie parametrov. Funguje to za využitia reguláreho výrazu,
ktorý popisuje jednotlivý typ parametru, ktorý je určený v poli *param_type*.
Ak je parameter validný, k XML elementu inštrukcie sa pridajú elementy popisujúce
argumenty, inak skript končí s návratovým kódom 23.

### Metóda *parse_prg_params* (private)
Kontrola argumentov skriptu.<br/>
Parametre:
- *argc* - počet argumentov
- *argv* - pole argumentov
Podpora len pre **--help**.

### Metóda *print_help* (private)
Výpis nápovedy skriptu.
