Syntax spuštění

Program se spouští v následující podobě: (./sheet značí umístění a název programu):

Úprava velikosti tabulky:

./sheet [-d DELIM] [Příkazy pro úpravu tabulky]

nebo zpracování dat:

./sheet [-d DELIM] [Selekce řádků] [Příkaz pro zpracování dat]

=====================================================================================
Úprava tabulky

Úpravy tabulky způsobují zvětšení či zmenšení tabulky, resp. řádků a sloupců. Příkazů pro úpravu tabulky může být zadáno více. V takovém případě budou zadány jako sekvence více argumentů příkazové řádky:

Příkazy pro úpravu tabulky:

    irow R - vloží řádek tabulky před řádek R > 0 (insert-row).
    arow - přidá nový řádek tabulky na konec tabulky (append-row).
    drow R - odstraní řádek číslo R > 0 (delete-row).
    drows N M - odstraní řádky N až M (N <= M). V případě N=M se příkaz chová stejně jako drow N.
    icol C - vloží prázdný sloupec před sloupec daný číslem C.
    acol - přidá prázdný sloupec za poslední sloupec.
    dcol C - odstraní sloupec číslo C.
    dcols N M - odstraní sloupce N až M (N <= M). V případě N=M se příkaz chová stejně jako dcol N.

=====================================================================================
Zpracování dat

Zpracování dat představuje úpravy obsahu jednotlivých buněk tabulky. Každé spuštění programu může obsahovat nejvíce jeden příkaz pro zpracování dat. Příkazy pro zpracování dat jsou následující:

    Příkazy, které jsou povinné pro úspěšné splnění projektu:
        cset C STR - do buňky ve sloupci C bude nastaven řetězec STR.
        tolower C - řetězec ve sloupci C bude převeden na malá písmena.
        toupper C - řetězec ve sloupce C bude převeden na velká písmena.
        round C - ve sloupci C zaokrouhlí číslo na celé číslo.
        int C - odstraní desetinnou část čísla ve sloupci C.
        copy N M - přepíše obsah buněk ve sloupci M hodnotami ze sloupce N.
        swap N M - zamění hodnoty buněk ve sloupcích N a M.
        move N M - přesune sloupec N před sloupec M.

=====================================================================================
Selekce řádků

Příkazy pro zpracování dat mohou být aplikovány nejen na celé tabulce, ale pouze na vybraných řádcích. Příkazy selekce takových řádků budou zadány na příkazovou řádku před příkazy pro zpracování dat:

    rows N M - procesor bude zpracovávat pouze řádky N až M včetně (N <= M). N=1 znamená zpracování od prvního řádku. Pokud je místo čísla M zadán znak - (pomlčka), ta reprezentuje poslední řádek vstupního souboru. Pokud je pomlčka také místo sloupce N, myslí se tím výběr pouze posledního řádku. Pokud není tento příkaz zadán, uvažuje se implicitně o všech řádcích.
    beginswith C STR - procesor bude zpracovávat pouze ty řádky, jejichž obsah buňky ve sloupci C začíná řetězcem STR.
    contains C STR - procesor bude zpracovávat pouze ty řádky, jejichž buňky ve sloupci C obsahují řetězec STR.

=====================================================================================
Omezení v projektu

Je zakázané použít následující funkce:

    volání z rodiny malloc a free - práce s dynamickou pamětí není v tomto projektu zapotřebí,
    volání z rodiny fopen, fclose, fscanf, ... - práce se soubory (dočasnými) není v tomto projektu žádoucí,
    volání qsort, lsearch, bsearch a hsearch - cílem je zamyslet se nad algoritmizací a strukturou dat.
    volání funkce exit - cílem projektu je naučit se vytvořit programové konstrukce, které dokáží zpracovat neočekávaný stav programu.

