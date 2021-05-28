Syntax spuštění

./sps [-d DELIM] CMD_SEQUENCE FILE

=====================================================================================
Příkazy pro změnu výběru

Výběr buněk může přesahovat hranice vstupní tabulky. V takovém případě bude tabulka rozšířena o požadovaný počet řádků a sloupců obsahující prázdné buňky až do aktuálního výběru.

    [R,C] - výběr buňky na řádku R a sloupci C.
    [R,_] - výběr celého řádku R.
    [_,C] - výběr celého sloupce C.
    [R1,C1,R2,C2] - výběr okna, tj. všech buněk na řádku R a sloupci C, pro které platí R1 <= R <= R2, C1 <= C <= C2. Pokud namísto čísla R2 resp. C2 bude pomlčka, nahrazuje tak maximální řádek resp. sloupec v tabulce.
    [_,_] - výběr celé tabulky.
    [min] - v již existujícím výběru buněk najde buňku s minimální numerickou hodnotou a výběr nastaví na ni.
    [max] - obdobně jako předchozí příkaz, ale najde buňku s maximální hodnotou.
    [find STR] - v již existujícím výběru buněk vybere první buňku, jejíž hodnota obsahuje podřetězec STR.
    [_] - obnoví výběr z dočasné proměnné (viz níže).

=====================================================================================
Příkazy pro úpravu struktury tabulky

    irow - vloží jeden prázdný řádek nalevo od vybraných buněk nad vybrané buňky.
    arow - přidá jeden prázdný řádek napravo od vybraných buněk pod vybrané buňky.
    drow - odstraní vybrané řádky.
    icol - vloží jeden prázdný sloupec nalevo od vybraných buněk.
    acol - přidá jeden prázdný sloupec napravo od vybraných buněk.
    dcol - odstraní vybrané sloupce.

=====================================================================================
Příkazy pro úpravu obsahu buněk

    set STR - nastaví hodnotu buňky na řetězec STR. Řetězec STR může být ohraničen uvozovkami a může obsahovat speciální znaky uvozené lomítkem (viz formát tabulky)
    clear - obsah vybraných buněk bude odstraněn (buňky budou mít prázdný obsah)
    swap [R,C] - vymění obsah vybrané buňky s buňkou na řádku R a sloupci C
    sum [R,C] - do buňky na řádku R a sloupci C uloží součet hodnot vybraných buněk (odpovídající formátu %g u printf). Vybrané buňky neobsahující číslo budou ignorovány (jako by vybrány nebyly).
    avg [R,C] - stejné jako sum, ale ukládá se aritmetický průměr z vybraných buněk
    count [R,C] - stejné jako sum, ale ukládá se počet neprázdných buněk z vybraných buněk
    len [R,C] - do buňky na řádku R a sloupci C uloží délku řetězce aktuálně vybrané buňky

=====================================================================================
Příkazy pro práci s dočasnými proměnnými

Tabulkový procesor umožnujě pracovat s 10 tzv. dočasnými proměnnými identifikovanými jako _0 až _9. Dočasné proměnné mají po spuštění programu prázdnou hodnotu. Jedna dočasná proměnná identikovaná jako podtržítko _ je určena pro výběr buněk (tj. pamatuje si, které buňky byly označeny).

    def _X - hodnota aktuální buňky bude nastavena do dočasné proměnné X (kde X může být hodnota 0 až 9)
    use _X - aktuální buňka bude nastavena na hodnotu z dočasné proměnné X (kde X identifikuje dočasnou proměnnou _0 až _9)
    inc _X - numerická hodnota v dočasné proměnné bude zvětšena o 1. Pokud dočasná proměnná neobsahuje číslo, bude výsledná hodnota proměnné nastavená na 1.
    [set] - nastaví aktuální výběr buněk do dočasné proměnné _ (ukládá se pouze, které buňky jsou vybrány, nikoliv jejich obsah)

